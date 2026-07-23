@echo off
cd /d "%~dp0"

echo ============================================
echo   MSPM0 FreeRTOS Template Tool
echo ============================================
echo.
echo   This tool will guide you to:
echo    1. Setup local toolchain paths
echo    2. Create a new project from template
echo.
echo ============================================
echo.

set BASH=

REM Search common Git Bash install paths
for %%d in (
    "D:\Program Files\Git\usr\bin"
    "D:\Program Files\Git\bin"
    "C:\Program Files\Git\usr\bin"
    "C:\Program Files\Git\bin"
    "C:\Program Files (x86)\Git\usr\bin"
    "C:\Program Files (x86)\Git\bin"
    "C:\msys64\usr\bin"
    "D:\msys64\usr\bin"
) do (
    if exist "%%~d\bash.exe" set "BASH=%%~d\bash.exe" & goto :run
)

REM Fallback: search PATH
for /f "delims=" %%i in ('where bash 2^>nul') do set "BASH=%%i" & goto :run

echo [ERROR] bash.exe not found!
echo Please install Git for Windows:
echo https://git-scm.com/download/win
pause
exit /b 1

:run
echo Using: %BASH%
echo.
"%BASH%" "%~dp0new_project.sh" %*
pause
