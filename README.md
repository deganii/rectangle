# Rectangle for Windows

A minimal window management tool for Windows inspired by [Rectangle](https://rectangleapp.com/) for macOS.

## Features

- **Global hotkeys** to snap windows to screen edges
- **Multi-monitor support** - works with the monitor containing the active window
- **System tray** - runs quietly in the background
- **Run on startup** - optional auto-start with Windows
- **Lightweight** - single ~50KB executable, no dependencies

## Hotkeys

### Enabled

| Shortcut | Action |
|----------|--------|
| `Ctrl+Shift+Left` | Snap to left half |
| `Ctrl+Shift+Right` | Snap to right half |
| `Ctrl+Shift+Down` | Snap to bottom half |
| `Ctrl+Shift+Up` | Maximize window |

### Disabled (available in code)

These hotkeys are commented out in `main.c` and can be enabled by uncommenting in three places: the enum, `RegisterHotkeys()`, and `HandleHotkey()`.

| Shortcut | Action |
|----------|--------|
| `Ctrl+Shift+Up` | Top half (conflicts with maximize) |
| `Ctrl+Shift+Q` | Top-left quarter |
| `Ctrl+Shift+W` | Top-right quarter |
| `Ctrl+Shift+A` | Bottom-left quarter |
| `Ctrl+Shift+S` | Bottom-right quarter |
| `Ctrl+Shift+C` | Center (80% size) |
| `Ctrl+Shift+R` | Restore window |

## Installation

1. Build the executable (see [build.md](build.md))
2. Place `rectangle.exe` anywhere you like
3. Run it - an icon appears in the system tray
4. (Optional) Right-click the tray icon and enable "Run on Startup"

## Usage

1. Focus any window you want to resize
2. Press one of the hotkey combinations
3. The window snaps to the specified position

The tool respects the monitor's work area, so windows won't overlap the taskbar.

## System Tray

Right-click the tray icon for options:

- **Run on Startup** - Toggle auto-start with Windows
- **Exit** - Close the application

## Uninstall

1. Right-click tray icon → Exit
2. Delete `rectangle.exe`
3. (If startup was enabled) The registry entry is at:
   `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run\RectangleWin`

## Building

See [build.md](build.md) for build instructions.

## License

Public domain / MIT - use as you wish.
