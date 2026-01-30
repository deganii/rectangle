@echo off
REM Build script for MinGW/GCC

gcc main.c -o rectangle.exe -luser32 -lshell32 -ladvapi32 -mwindows -O2

if %ERRORLEVEL% EQU 0 (
    echo Build successful: rectangle.exe
) else (
    echo Build failed
)
