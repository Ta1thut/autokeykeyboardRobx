@echo off
REM Build with CMake
REM   scripts\build_cmake.bat          -> Visual Studio (if installed)
REM   scripts\build_cmake.bat mingw    -> MinGW Makefiles
REM Output: build\Release\AutoKey.exe (VS) or build\AutoKey.exe (MinGW)
setlocal
cd /d "%~dp0.."

where cmake >nul 2>nul || (echo [ERROR] cmake not found. & goto :error)

set GEN=
if /i "%~1"=="mingw" set GEN=-G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

cmake -S . -B build %GEN% || goto :error
cmake --build build --config Release || goto :error

echo.
echo Build OK
exit /b 0

:error
echo.
echo Build FAILED
exit /b 1
