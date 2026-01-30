@echo off
REM Build script for MSVC (Visual Studio)
REM Run from "Developer Command Prompt for VS"

cl main.c /O2 /W3 /link user32.lib shell32.lib advapi32.lib /out:rectangle.exe

if %ERRORLEVEL% EQU 0 (
    echo Build successful: rectangle.exe
    del main.obj 2>nul
) else (
    echo Build failed
)
