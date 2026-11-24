@echo off
REM Build with MinGW-w64 (g++), no CMake needed
REM Output: build\AutoKey.exe
setlocal
cd /d "%~dp0.."

where g++ >nul 2>nul || (echo [ERROR] g++ not found. Install MinGW-w64 and add its bin folder to PATH. & goto :error)
where windres >nul 2>nul || (echo [ERROR] windres not found. It ships with MinGW-w64. & goto :error)

if not exist build mkdir build

echo [1/2] Compile resources...
windres -I src -I resources resources\app.rc -O coff -o build\app_res.o || goto :error

echo [2/2] Compile + link...
g++ -std=c++17 -O2 -Wall -Wextra ^
    -Isrc ^
    src\main.cpp src\main_window.cpp src\macro_runner.cpp src\key_parser.cpp src\input_sender.cpp ^
    build\app_res.o ^
    -o build\AutoKey.exe ^
    -mwindows -static -static-libgcc -static-libstdc++ || goto :error

echo.
echo Build OK  -^>  build\AutoKey.exe
exit /b 0

:error
echo.
echo Build FAILED
exit /b 1
