# Windows Rectangle Clone - Research Summary

A minimalist window management tool for Windows with configurable hotkeys.

---

## Approach Comparison

| Approach | Complexity | Dependencies | Single EXE | Customizable |
|----------|------------|--------------|------------|--------------|
| C/C++ Native | Medium | None (Win32 API) | Yes | Config file |
| AutoHotkey | Low | AHK runtime (~3MB) | Yes (compiled) | Script file |
| PowerShell | Low | Built-in | No (script) | Script file |
| Python + pywin32 | Medium | Python + packages | Yes (PyInstaller) | Config file |
| FancyZones | None | PowerToys (~200MB) | N/A | GUI |

---

## Option 1: C/C++ Native (Recommended for Minimal Moving Parts)

**Pros:**
- Single small EXE (~50-100KB)
- Zero runtime dependencies
- Direct Win32 API access
- Fast startup, low memory

**Cons:**
- More code to write initially
- Requires C/C++ compiler (MSVC, MinGW, or Clang)

### Core Win32 APIs Needed

```c
// Register global hotkeys
RegisterHotKey(NULL, id, MOD_WIN | MOD_ALT, VK_LEFT);

// Get foreground window
HWND hwnd = GetForegroundWindow();

// Move/resize window
SetWindowPos(hwnd, NULL, x, y, width, height, SWP_NOZORDER);
// or
MoveWindow(hwnd, x, y, width, height, TRUE);

// Get monitor info
GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &mi);

// Message loop
while (GetMessage(&msg, NULL, 0, 0)) {
    if (msg.message == WM_HOTKEY) {
        // Handle hotkey based on msg.wParam (hotkey ID)
    }
}
```

### Minimal Implementation Structure

```
rectangle-win/
├── main.c          # ~200-300 lines
├── config.ini      # Hotkey mappings
└── build.bat       # cl main.c /O2 /link user32.lib
```

**References:**
- [RegisterHotKey - Microsoft Learn](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-registerhotkey)
- [Microsoft RegisterHotKey Sample](https://github.com/microsoft/Windows-classic-samples/blob/main/Samples/Win7Samples/winui/RegisterHotKey/RegisterHotKey.cpp)
- [SetWindowPos - pinvoke.net](https://pinvoke.net/default.aspx/user32/SetWindowPos.html)

---

## Option 2: AutoHotkey (Simplest to Write)

**Pros:**
- Very concise syntax (~50 lines for basic functionality)
- Can compile to standalone EXE
- Large community with examples
- Easy to modify

**Cons:**
- Requires AHK v2 runtime (or compiled EXE includes it)
- Slightly larger binary when compiled (~1MB)

### Example Script (AHK v2)

```autohotkey
; Win+Alt+Left = Left half
#!Left::WinMove(0, 0, A_ScreenWidth/2, A_ScreenHeight, "A")

; Win+Alt+Right = Right half
#!Right::WinMove(A_ScreenWidth/2, 0, A_ScreenWidth/2, A_ScreenHeight, "A")

; Win+Alt+Up = Top half
#!Up::WinMove(0, 0, A_ScreenWidth, A_ScreenHeight/2, "A")

; Win+Alt+Down = Bottom half
#!Down::WinMove(0, A_ScreenHeight/2, A_ScreenWidth, A_ScreenHeight/2, "A")

; Win+Alt+C = Center
#!c::WinMove((A_ScreenWidth-800)/2, (A_ScreenHeight-600)/2, 800, 600, "A")

; Win+Alt+F = Maximize
#!f::WinMaximize("A")
```

**References:**
- [Window Positioning with AutoHotkey](https://www.damirscorner.com/blog/posts/20200522-PositioningWithAutoHotkey.html)
- [AutoHotkey Window Position Gist](https://gist.github.com/brianpeiris/a770c89a74464418d3ba)
- [Simple Window Manager - AHK Forums](https://www.autohotkey.com/boards/viewtopic.php?t=69089)

---

## Option 3: PowerShell (No Installation Required)

**Pros:**
- Built into Windows (no install)
- Quick to prototype
- Can be run from Task Scheduler

**Cons:**
- Slow startup (~1-2 seconds)
- No clean hotkey registration (needs AHK or external trigger)
- More verbose code

### Example Script

```powershell
Add-Type @"
using System;
using System.Runtime.InteropServices;
public class Win32 {
    [DllImport("user32.dll")]
    public static extern IntPtr GetForegroundWindow();
    [DllImport("user32.dll")]
    public static extern bool MoveWindow(IntPtr hWnd, int X, int Y, int W, int H, bool repaint);
}
"@

$screen = [System.Windows.Forms.Screen]::PrimaryScreen.WorkingArea
$hwnd = [Win32]::GetForegroundWindow()

# Move to left half
[Win32]::MoveWindow($hwnd, 0, 0, $screen.Width/2, $screen.Height, $true)
```

**References:**
- [Positioning Windows with PowerShell](https://dadoverflow.com/2018/11/18/positioning-windows-with-powershell/)
- [Weekend Scripter: Manage Window Placement](https://devblogs.microsoft.com/scripting/weekend-scripter-manage-window-placement-by-using-pinvoke/)

---

## Option 4: Python + pywin32

**Pros:**
- Readable code
- Good library ecosystem
- Can package to EXE with PyInstaller

**Cons:**
- Requires Python or creates large EXE (~15-30MB)
- More dependencies

### Example Code

```python
import win32gui
import win32con
import win32api
import ctypes

def move_window_left_half():
    hwnd = win32gui.GetForegroundWindow()
    monitor = win32api.GetMonitorInfo(win32api.MonitorFromWindow(hwnd))
    work_area = monitor['Work']
    width = (work_area[2] - work_area[0]) // 2
    height = work_area[3] - work_area[1]
    win32gui.SetWindowPos(hwnd, None, work_area[0], work_area[1],
                          width, height, win32con.SWP_NOZORDER)

# Register hotkey: Win+Alt+Left
ctypes.windll.user32.RegisterHotKey(None, 1, 0x0001 | 0x0008, 0x25)

# Message loop
msg = ctypes.wintypes.MSG()
while ctypes.windll.user32.GetMessageW(ctypes.byref(msg), None, 0, 0):
    if msg.message == 0x0312:  # WM_HOTKEY
        if msg.wParam == 1:
            move_window_left_half()
```

**References:**
- [Guide to Windows Hotkey Automation in Python](https://christoph-muessig.medium.com/a-guide-to-windows-hotkey-automatisation-in-python-327fb134df8b)
- [win32gui SetWindowPos](https://timgolden.me.uk/pywin32-docs/win32gui__SetWindowPos_meth.html)
- [Controlling Window Position in Python](https://en.ittrip.xyz/python/control-win-pos-size-python)

---

## Option 5: Existing Tools

### FancyZones (Microsoft PowerToys)
- Part of PowerToys suite
- GUI for defining zones
- Drag windows to zones or use Win+Arrow
- **Downside:** Installs full PowerToys (~200MB)

### GlazeWM
- Open source tiling window manager
- Automatic tiling (like i3/dwm)
- Keyboard-driven workflow
- Active development
- **Downside:** Full tiling manager, may be overkill

### Bug.n
- Written in AutoHotkey
- Tiling window manager
- Multi-monitor support
- **Downside:** Full tiling manager

**References:**
- [Rectangle Alternatives for Windows](https://alternativeto.net/software/rectangle-windows-manager-/?platform=windows)
- [GlazeWM - MakeUseOf](https://www.makeuseof.com/glazewm-open-source-window-manager-for-windows/)
- [10 Tiling Window Managers for Windows](https://medevel.com/tiling-window-manager-for-windows-1001/)

---

## Recommendation

For **minimal moving parts**, go with **C/C++ Native**:

1. Single ~50KB EXE with no dependencies
2. Instant startup, runs in system tray
3. Reads hotkeys from simple INI/JSON config
4. Uses only Win32 API (user32.dll - always present on Windows)

The implementation would be roughly 200-400 lines of C code:
- `RegisterHotKey()` for each configured shortcut
- Message loop checking `WM_HOTKEY`
- `GetForegroundWindow()` + `SetWindowPos()` for window manipulation
- `GetMonitorInfo()` for multi-monitor support
- Simple INI parser for config

**Build with:** `cl main.c /O2 /link user32.lib` (MSVC) or `gcc main.c -o rectangle.exe -luser32 -mwindows` (MinGW)
