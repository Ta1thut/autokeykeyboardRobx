#include "input_sender.h"

#include "win_common.h"

namespace autokey {
namespace {

bool IsExtendedKey(int vk) {
    switch (vk) {
        case VK_UP: case VK_DOWN: case VK_LEFT: case VK_RIGHT:
        case VK_INSERT: case VK_DELETE: case VK_HOME: case VK_END:
        case VK_PRIOR: case VK_NEXT:
            return true;
        default:
            return false;
    }
}

}  // namespace

void SendKey(int vk, bool down) {
    INPUT in = {};

    if (vk == VK_LBUTTON || vk == VK_RBUTTON) {
        in.type = INPUT_MOUSE;
        if (vk == VK_LBUTTON)
            in.mi.dwFlags = down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
        else
            in.mi.dwFlags = down ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
        SendInput(1, &in, sizeof(INPUT));
        return;
    }

    in.type = INPUT_KEYBOARD;
    in.ki.wScan = static_cast<WORD>(MapVirtualKeyW(static_cast<UINT>(vk), MAPVK_VK_TO_VSC));
    in.ki.dwFlags = KEYEVENTF_SCANCODE;
    if (!down) in.ki.dwFlags |= KEYEVENTF_KEYUP;
    if (IsExtendedKey(vk)) in.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    SendInput(1, &in, sizeof(INPUT));
}

}  // namespace autokey
