#ifndef AUTOKEY_WIN_COMMON_H
#define AUTOKEY_WIN_COMMON_H

// ตั้งค่าที่ต้องกำหนดก่อน include <windows.h> เสมอ
// (ใส่ไว้ที่เดียว เพื่อให้ build ผ่านไม่ว่าจะใช้ CMake, สคริปต์ .bat หรือคอมไพล์มือ)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601  // Windows 7 ขึ้นไป (GetTickCount64, MOD_NOREPEAT)
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#endif  // AUTOKEY_WIN_COMMON_H
