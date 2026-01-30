# Rectangle for Windows - Implementation Plan

## Overview
A minimal Win32 application that provides Rectangle-like window management via global hotkeys.

## Features
1. **Global Hotkeys** - Register system-wide hotkeys for window positioning
2. **Window Positioning** - Move/resize active window to predefined positions
3. **Multi-Monitor Support** - Respect current monitor's work area
4. **System Tray** - Minimize to tray, right-click menu for exit
5. **Run on Startup** - Optional registry entry for auto-start
6. **Config File** - INI file for customizable hotkeys

## Hotkeys

### Enabled
| Shortcut | Action |
|----------|--------|
| Ctrl+Shift+Left | Left half |
| Ctrl+Shift+Right | Right half |
| Ctrl+Shift+Down | Bottom half |
| Ctrl+Shift+Up | Maximize |

### Disabled (commented out in code)
| Shortcut | Action |
|----------|--------|
| Ctrl+Shift+Up | Top half (conflicts with maximize) |
| Ctrl+Shift+Q | Top-left quarter |
| Ctrl+Shift+W | Top-right quarter |
| Ctrl+Shift+A | Bottom-left quarter |
| Ctrl+Shift+S | Bottom-right quarter |
| Ctrl+Shift+C | Center (80% size) |
| Ctrl+Shift+R | Restore |

## File Structure
```
rectangle/
├── main.c           # Main application (~400 lines)
├── config.ini       # Hotkey configuration
├── build.bat        # MSVC build script
├── build-mingw.bat  # MinGW build script
├── plan.md          # This file
└── research.md      # Research notes
```

## Implementation Tasks

### 1. Core Window Management
- [x] Get foreground window handle
- [x] Get monitor work area (excluding taskbar)
- [x] SetWindowPos for move/resize
- [x] Handle maximized windows (restore first)

### 2. Hotkey System
- [x] RegisterHotKey for each action
- [x] Message loop with WM_HOTKEY handling
- [x] Hotkey ID to action mapping
- [x] Cleanup with UnregisterHotKey on exit

### 3. System Tray
- [x] Create tray icon with Shell_NotifyIcon
- [x] Context menu (Enable Startup / Exit)
- [x] Handle WM_TRAYICON messages
- [x] Tooltip showing app name

### 4. Run on Startup
- [x] Registry key: HKCU\Software\Microsoft\Windows\CurrentVersion\Run
- [x] Toggle via tray menu
- [x] Check current state on startup

### 5. Configuration (Optional - Phase 2)
- [ ] INI file parser
- [ ] Custom hotkey mappings
- [ ] Reload config option

## Build Instructions

### Using MSVC (Visual Studio)
```bat
cl main.c /O2 /W3 /link user32.lib shell32.lib advapi32.lib /out:rectangle.exe
```

### Using MinGW
```bat
gcc main.c -o rectangle.exe -luser32 -lshell32 -ladvapi32 -mwindows -O2
```

## Technical Notes

### Win32 APIs Used
- `RegisterHotKey` / `UnregisterHotKey` - Global hotkey registration
- `GetForegroundWindow` - Get active window
- `SetWindowPos` / `MoveWindow` - Position windows
- `MonitorFromWindow` / `GetMonitorInfo` - Multi-monitor support
- `Shell_NotifyIcon` - System tray
- `RegOpenKeyEx` / `RegSetValueEx` / `RegDeleteValue` - Startup registry
- `GetWindowPlacement` / `ShowWindow` - Handle maximized state

### Hotkey Modifiers
- `MOD_WIN` = 0x0008
- `MOD_ALT` = 0x0001
- `MOD_CONTROL` = 0x0002
- `MOD_SHIFT` = 0x0004
- `MOD_NOREPEAT` = 0x4000 (prevent repeat when held)
