# Architecture

This document explains the structure and design of Rectangle for Windows.

## Overview

Rectangle is a single-file Win32 application (~250 lines of C) that registers global hotkeys and moves windows in response. It runs as a background process with a system tray icon.

```
┌─────────────────────────────────────────────────────────┐
│                    Windows OS                           │
│  ┌───────────────┐  ┌───────────────┐  ┌─────────────┐ │
│  │ Keyboard Input│  │  user32.dll   │  │ advapi32.dll│ │
│  └───────┬───────┘  └───────┬───────┘  └──────┬──────┘ │
│          │                  │                 │        │
│          ▼                  ▼                 ▼        │
│  ┌─────────────────────────────────────────────────┐   │
│  │              rectangle.exe                       │   │
│  │  ┌──────────┐ ┌──────────┐ ┌──────────────────┐ │   │
│  │  │ Hotkey   │ │ Window   │ │ Startup Registry │ │   │
│  │  │ Handler  │ │ Mover    │ │ Manager          │ │   │
│  │  └──────────┘ └──────────┘ └──────────────────┘ │   │
│  │  ┌──────────────────────────────────────────┐   │   │
│  │  │           System Tray Icon               │   │   │
│  │  └──────────────────────────────────────────┘   │   │
│  └─────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

## File Structure

```
main.c              # Single source file containing all logic
├── wWinMain()      # Entry point
├── WndProc()       # Message handler
├── RegisterHotkeys()   # Hotkey registration
├── HandleHotkey()      # Hotkey action dispatcher
├── MoveActiveWindow()  # Window positioning
├── Tray functions      # System tray management
└── Startup functions   # Registry read/write
```

## Core Components

### 1. Entry Point (`wWinMain`)

```
wWinMain()
    │
    ├── CreateMutex()          # Prevent multiple instances
    ├── RegisterClassEx()      # Register window class
    ├── CreateWindowEx()       # Create hidden message window
    ├── RegisterHotkeys()      # Register global hotkeys
    ├── CreateTrayIcon()       # Add system tray icon
    │
    └── Message Loop ──────────► GetMessage()
                                     │
                                     ▼
                               DispatchMessage()
                                     │
                                     ▼
                                 WndProc()
```

### 2. Message Handler (`WndProc`)

Handles three message types:

| Message | Source | Action |
|---------|--------|--------|
| `WM_HOTKEY` | Keyboard | Call `HandleHotkey()` with hotkey ID |
| `WM_TRAYICON` | Tray icon click | Show context menu |
| `WM_COMMAND` | Menu selection | Toggle startup or exit |

### 3. Hotkey System

**Registration:**
```c
RegisterHotKey(hWnd, HK_LEFT, MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT, VK_LEFT);
```

- `hWnd` - Window to receive `WM_HOTKEY` messages
- `HK_LEFT` - Unique ID (1-4 in our case)
- `MOD_*` - Modifier keys (Ctrl+Shift)
- `MOD_NOREPEAT` - Don't repeat when held down
- `VK_LEFT` - Virtual key code

**Hotkey IDs:**
```c
enum {
    HK_LEFT = 1,    // Ctrl+Shift+Left
    HK_RIGHT,       // Ctrl+Shift+Right
    HK_BOTTOM,      // Ctrl+Shift+Down
    HK_MAXIMIZE,    // Ctrl+Shift+Up
    /* Disabled - uncomment to enable:
    HK_TOP, HK_TOPLEFT, HK_TOPRIGHT,
    HK_BOTTOMLEFT, HK_BOTTOMRIGHT,
    HK_CENTER, HK_RESTORE, */
    HK_COUNT
};
```

### 4. Window Positioning

**Flow:**
```
HandleHotkey(id)
    │
    ├── GetForegroundWindow()      # Get active window
    ├── GetMonitorWorkArea()       # Get monitor bounds (excludes taskbar)
    │       │
    │       ├── MonitorFromWindow()    # Find which monitor
    │       └── GetMonitorInfo()       # Get work area rect
    │
    └── MoveActiveWindow(x, y, w, h)
            │
            ├── GetWindowPlacement()   # Check if maximized
            ├── ShowWindow(SW_RESTORE) # Restore if needed
            └── SetWindowPos()         # Move and resize
```

**Position calculations:**
```
Monitor Work Area
┌────────────────────────────────┐
│ mx, my                         │
│  ┌──────────┬─────────────────┐│
│  │          │                 ││
│  │   LEFT   │     RIGHT       ││
│  │  mw/2    │     mw/2        ││
│  │          │                 ││
│  ├──────────┴─────────────────┤│
│  │         BOTTOM             ││
│  │          mh/2              ││
│  └────────────────────────────┘│
│                          mw, mh│
└────────────────────────────────┘

mx = work area left edge
my = work area top edge
mw = work area width
mh = work area height
```

### 5. System Tray

**Structure:**
```c
NOTIFYICONDATA {
    hWnd            // Window for callbacks
    uID             // Icon identifier
    uFlags          // NIF_ICON | NIF_MESSAGE | NIF_TIP
    uCallbackMessage// WM_TRAYICON (custom message)
    hIcon           // Icon handle
    szTip           // Tooltip text
}
```

**Operations:**
- `Shell_NotifyIcon(NIM_ADD, ...)` - Create icon
- `Shell_NotifyIcon(NIM_DELETE, ...)` - Remove icon
- `TrackPopupMenu()` - Show right-click menu

### 6. Startup Registry

**Registry location:**
```
HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run
```

**Value:**
- Name: `RectangleWin`
- Type: `REG_SZ`
- Data: Full path to `rectangle.exe`

**Functions:**
- `IsStartupEnabled()` - Check if registry value exists
- `SetStartupEnabled(true)` - Write exe path to registry
- `SetStartupEnabled(false)` - Delete registry value

## Win32 APIs Used

| API | Library | Purpose |
|-----|---------|---------|
| `RegisterHotKey` | user32 | Register global hotkey |
| `UnregisterHotKey` | user32 | Unregister hotkey |
| `GetForegroundWindow` | user32 | Get active window |
| `SetWindowPos` | user32 | Move/resize window |
| `GetWindowPlacement` | user32 | Check window state |
| `ShowWindow` | user32 | Maximize/restore |
| `MonitorFromWindow` | user32 | Get monitor handle |
| `GetMonitorInfo` | user32 | Get monitor dimensions |
| `Shell_NotifyIcon` | shell32 | System tray icon |
| `TrackPopupMenu` | user32 | Context menu |
| `RegOpenKeyEx` | advapi32 | Open registry key |
| `RegSetValueEx` | advapi32 | Write registry value |
| `RegDeleteValue` | advapi32 | Delete registry value |
| `RegQueryValueEx` | advapi32 | Read registry value |

## Memory and Resources

- **No heap allocations** - All data is stack or static
- **Single mutex** - Prevents multiple instances
- **One tray icon** - Cleaned up on exit
- **Four hotkeys** - Unregistered on exit

## Extending

To add a new hotkey:

1. Add ID to the enum:
   ```c
   enum { HK_LEFT = 1, ..., HK_NEWACTION, HK_COUNT };
   ```

2. Register in `RegisterHotkeys()`:
   ```c
   RegisterHotKey(g_hWnd, HK_NEWACTION, mod, VK_KEY);
   ```

3. Handle in `HandleHotkey()`:
   ```c
   case HK_NEWACTION:
       MoveActiveWindow(...);
       break;
   ```
