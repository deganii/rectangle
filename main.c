/*
 * Rectangle for Windows
 * A minimal window manager with hotkey support
 *
 * Build (MSVC):  cl main.c /O2 /W3 /link user32.lib shell32.lib advapi32.lib /out:rectangle.exe
 * Build (MinGW): gcc main.c -o rectangle.exe -luser32 -lshell32 -ladvapi32 -mwindows -O2
 */

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <shellapi.h>
#include <stdbool.h>

/* Tray icon message */
#define WM_TRAYICON (WM_USER + 1)
#define IDI_TRAYICON 1

/* Menu IDs */
#define IDM_STARTUP 1001
#define IDM_EXIT    1002

/* Hotkey IDs */
enum {
    HK_LEFT = 1,
    HK_RIGHT,
    HK_BOTTOM,
    HK_MAXIMIZE,
    /* Disabled - uncomment to enable */
    // HK_TOP,
    // HK_TOPLEFT,
    // HK_TOPRIGHT,
    // HK_BOTTOMLEFT,
    // HK_BOTTOMRIGHT,
    // HK_CENTER,
    // HK_RESTORE,
    HK_COUNT
};

/* Globals */
static HINSTANCE g_hInstance;
static HWND g_hWnd;
static NOTIFYICONDATA g_nid;
static const wchar_t* APP_NAME = L"Rectangle";
static const wchar_t* STARTUP_KEY = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const wchar_t* STARTUP_VALUE = L"RectangleWin";

/* Forward declarations */
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void RegisterHotkeys(void);
void UnregisterHotkeys(void);
void HandleHotkey(int id);
void MoveActiveWindow(int x, int y, int w, int h);
void GetMonitorWorkArea(HWND hwnd, RECT* rc);
void CreateTrayIcon(HWND hwnd);
void RemoveTrayIcon(void);
void ShowContextMenu(HWND hwnd);
bool IsStartupEnabled(void);
void SetStartupEnabled(bool enable);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    g_hInstance = hInstance;

    /* Check for existing instance */
    HANDLE hMutex = CreateMutex(NULL, TRUE, L"RectangleWinMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMutex);
        return 0;
    }

    /* Register window class */
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"RectangleWinClass";
    RegisterClassEx(&wc);

    /* Create hidden window for message handling */
    g_hWnd = CreateWindowEx(0, wc.lpszClassName, APP_NAME, 0,
                            0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);
    if (!g_hWnd) {
        return 1;
    }

    /* Setup */
    RegisterHotkeys();
    CreateTrayIcon(g_hWnd);

    /* Message loop */
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    /* Cleanup */
    UnregisterHotkeys();
    RemoveTrayIcon();
    CloseHandle(hMutex);

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_HOTKEY:
            HandleHotkey((int)wParam);
            return 0;

        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP) {
                ShowContextMenu(hwnd);
            }
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_STARTUP:
                    SetStartupEnabled(!IsStartupEnabled());
                    break;
                case IDM_EXIT:
                    PostQuitMessage(0);
                    break;
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void RegisterHotkeys(void) {
    UINT mod = MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT;

    RegisterHotKey(g_hWnd, HK_LEFT,     mod, VK_LEFT);   /* Ctrl+Shift+Left */
    RegisterHotKey(g_hWnd, HK_RIGHT,    mod, VK_RIGHT);  /* Ctrl+Shift+Right */
    RegisterHotKey(g_hWnd, HK_BOTTOM,   mod, VK_DOWN);   /* Ctrl+Shift+Down */
    RegisterHotKey(g_hWnd, HK_MAXIMIZE, mod, VK_UP);     /* Ctrl+Shift+Up */

    /* Disabled - uncomment to enable (also uncomment in enum and HandleHotkey) */
    // RegisterHotKey(g_hWnd, HK_TOP,         mod, VK_UP);     /* Ctrl+Shift+Up (conflicts with maximize) */
    // RegisterHotKey(g_hWnd, HK_TOPLEFT,     mod, 'Q');       /* Ctrl+Shift+Q */
    // RegisterHotKey(g_hWnd, HK_TOPRIGHT,    mod, 'W');       /* Ctrl+Shift+W */
    // RegisterHotKey(g_hWnd, HK_BOTTOMLEFT,  mod, 'A');       /* Ctrl+Shift+A */
    // RegisterHotKey(g_hWnd, HK_BOTTOMRIGHT, mod, 'S');       /* Ctrl+Shift+S */
    // RegisterHotKey(g_hWnd, HK_CENTER,      mod, 'C');       /* Ctrl+Shift+C */
    // RegisterHotKey(g_hWnd, HK_RESTORE,     mod, 'R');       /* Ctrl+Shift+R */
}

void UnregisterHotkeys(void) {
    for (int i = 1; i < HK_COUNT; i++) {
        UnregisterHotKey(g_hWnd, i);
    }
}

void GetMonitorWorkArea(HWND hwnd, RECT* rc) {
    HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { .cbSize = sizeof(MONITORINFO) };
    GetMonitorInfo(hMon, &mi);
    *rc = mi.rcWork;  /* Work area excludes taskbar */
}

void MoveActiveWindow(int x, int y, int w, int h) {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return;

    /* Skip if it's the desktop or shell */
    wchar_t className[256];
    GetClassName(hwnd, className, 256);
    if (wcscmp(className, L"Progman") == 0 ||
        wcscmp(className, L"WorkerW") == 0 ||
        wcscmp(className, L"Shell_TrayWnd") == 0) {
        return;
    }

    /* Restore if maximized */
    WINDOWPLACEMENT wp = { .length = sizeof(WINDOWPLACEMENT) };
    GetWindowPlacement(hwnd, &wp);
    if (wp.showCmd == SW_SHOWMAXIMIZED) {
        ShowWindow(hwnd, SW_RESTORE);
    }

    SetWindowPos(hwnd, NULL, x, y, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
}

void HandleHotkey(int id) {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return;

    RECT wa;
    GetMonitorWorkArea(hwnd, &wa);
    int mw = wa.right - wa.left;   /* Monitor width */
    int mh = wa.bottom - wa.top;   /* Monitor height */
    int mx = wa.left;              /* Monitor X offset */
    int my = wa.top;               /* Monitor Y offset */

    switch (id) {
        case HK_LEFT:        /* Left half */
            MoveActiveWindow(mx, my, mw / 2, mh);
            break;

        case HK_RIGHT:       /* Right half */
            MoveActiveWindow(mx + mw / 2, my, mw / 2, mh);
            break;

        case HK_BOTTOM:      /* Bottom half */
            MoveActiveWindow(mx, my + mh / 2, mw, mh / 2);
            break;

        case HK_MAXIMIZE:    /* Maximize */
            ShowWindow(hwnd, SW_MAXIMIZE);
            break;

        /* Disabled - uncomment to enable (also uncomment in enum and RegisterHotkeys)
        case HK_TOP:
            MoveActiveWindow(mx, my, mw, mh / 2);
            break;

        case HK_TOPLEFT:
            MoveActiveWindow(mx, my, mw / 2, mh / 2);
            break;

        case HK_TOPRIGHT:
            MoveActiveWindow(mx + mw / 2, my, mw / 2, mh / 2);
            break;

        case HK_BOTTOMLEFT:
            MoveActiveWindow(mx, my + mh / 2, mw / 2, mh / 2);
            break;

        case HK_BOTTOMRIGHT:
            MoveActiveWindow(mx + mw / 2, my + mh / 2, mw / 2, mh / 2);
            break;

        case HK_CENTER: {
            int cw = mw * 80 / 100;
            int ch = mh * 80 / 100;
            MoveActiveWindow(mx + (mw - cw) / 2, my + (mh - ch) / 2, cw, ch);
            break;
        }

        case HK_RESTORE:
            ShowWindow(hwnd, SW_RESTORE);
            break;
        */
    }
}

void CreateTrayIcon(HWND hwnd) {
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hwnd;
    g_nid.uID = IDI_TRAYICON;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcscpy_s(g_nid.szTip, sizeof(g_nid.szTip)/sizeof(wchar_t), APP_NAME);
    Shell_NotifyIcon(NIM_ADD, &g_nid);
}

void RemoveTrayIcon(void) {
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
}

void ShowContextMenu(HWND hwnd) {
    HMENU hMenu = CreatePopupMenu();

    /* Startup toggle with checkmark */
    UINT startupFlags = MF_STRING | (IsStartupEnabled() ? MF_CHECKED : MF_UNCHECKED);
    AppendMenu(hMenu, startupFlags, IDM_STARTUP, L"Run on Startup");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, IDM_EXIT, L"Exit");

    /* Show menu at cursor position */
    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(hwnd);  /* Required for menu to close properly */
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
    DestroyMenu(hMenu);
}

bool IsStartupEnabled(void) {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, STARTUP_KEY, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return false;
    }

    wchar_t value[MAX_PATH];
    DWORD size = sizeof(value);
    DWORD type;
    bool enabled = (RegQueryValueEx(hKey, STARTUP_VALUE, NULL, &type, (LPBYTE)value, &size) == ERROR_SUCCESS);

    RegCloseKey(hKey);
    return enabled;
}

void SetStartupEnabled(bool enable) {
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, STARTUP_KEY, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        return;
    }

    if (enable) {
        wchar_t exePath[MAX_PATH];
        GetModuleFileName(NULL, exePath, MAX_PATH);
        RegSetValueEx(hKey, STARTUP_VALUE, 0, REG_SZ, (LPBYTE)exePath, (DWORD)((wcslen(exePath) + 1) * sizeof(wchar_t)));
    } else {
        RegDeleteValue(hKey, STARTUP_VALUE);
    }

    RegCloseKey(hKey);
}
