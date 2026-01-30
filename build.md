# Build Instructions

## Tool Paths

| Tool | Path |
|------|------|
| Visual Studio 2026 | `C:\Program Files\Microsoft Visual Studio\18\Community` |
| CMake | `C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe` |

## Building with MSVC

Open **Developer Command Prompt for VS 2026** or run:

```bat
"C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"
```

Then build:

```bat
cd C:\dev\rectangle
build.bat
```

Or manually:

```bat
cl main.c /O2 /W3 /link user32.lib shell32.lib advapi32.lib /out:rectangle.exe
```

## Building with MinGW

```bat
cd C:\dev\rectangle
build-mingw.bat
```

Or manually:

```bat
gcc main.c -o rectangle.exe -luser32 -lshell32 -ladvapi32 -mwindows -O2
```

## Output

- `rectangle.exe` (~50KB, no dependencies)
