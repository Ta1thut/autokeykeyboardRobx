cmake_minimum_required(VERSION 3.16)
project(AutoKey VERSION 1.0.0 LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(AutoKey WIN32
    src/main.cpp
    src/main_window.cpp
    src/macro_runner.cpp
    src/key_parser.cpp
    src/input_sender.cpp
    resources/app.rc
)

target_include_directories(AutoKey PRIVATE src resources)
# UNICODE / _WIN32_WINNT ถูกกำหนดใน src/win_common.h แล้ว

if(MSVC)
    # รวม runtime เข้า exe ให้ไม่ต้องลง VC++ Redistributable
    set_property(TARGET AutoKey PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_compile_options(AutoKey PRIVATE /W4 /utf-8 /EHsc)
    # manifest ฝังมาจาก resources/app.rc แล้ว ไม่ให้ linker สร้างซ้ำ
    target_link_options(AutoKey PRIVATE /MANIFEST:NO)
else()
    target_compile_options(AutoKey PRIVATE -Wall -Wextra)
    target_link_options(AutoKey PRIVATE -static -static-libgcc -static-libstdc++)
endif()
