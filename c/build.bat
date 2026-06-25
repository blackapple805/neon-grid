@echo off
REM build.bat - compile and run neon-life on Windows.
REM Requires gcc on PATH (from w64devkit). Double-click or run from a terminal.

setlocal
cd /d "%~dp0"

echo Building neon-life...
gcc -std=c11 -Wall -Wextra -O2 src\main.c src\life.c src\palette.c -o neon-life.exe
if errorlevel 1 (
    echo.
    echo BUILD FAILED. Is gcc on your PATH? Open the w64devkit terminal,
    echo or add w64devkit\bin to PATH, then try again.
    pause
    exit /b 1
)

echo Build OK. Launching ^(press Q to quit^)...
echo.
neon-life.exe
endlocal

