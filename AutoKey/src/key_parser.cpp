#include "key_parser.h"

#include "win_common.h"

#include <cwctype>

namespace autokey {
namespace {

constexpr int kMaxWaitMs = 3600000;  // 1 ชั่วโมง

struct KeyName {
    const wchar_t* name;
    int vk;
};

const KeyName kNames[] = {
    {L"SPACE", VK_SPACE},   {L"ENTER", VK_RETURN},   {L"TAB", VK_TAB},
    {L"ESC", VK_ESCAPE},    {L"SHIFT", VK_SHIFT},    {L"CTRL", VK_CONTROL},
    {L"ALT", VK_MENU},      {L"BACKSPACE", VK_BACK}, {L"DELETE", VK_DELETE},
    {L"INSERT", VK_INSERT}, {L"HOME", VK_HOME},      {L"END", VK_END},
    {L"PGUP", VK_PRIOR},    {L"PGDN", VK_NEXT},
    {L"UP", VK_UP},         {L"DOWN", VK_DOWN},      {L"LEFT", VK_LEFT},
    {L"RIGHT", VK_RIGHT},
    {L"LMB", VK_LBUTTON},   {L"RMB", VK_RBUTTON},
};

// แปลงตัวเลขล้วนเป็น int (ตัวที่ไม่ใช่เลขหยุดอ่าน, ค่ามากเกินจะถูกจำกัดไว้ ไม่ overflow)
int ParseUInt(const std::wstring& s, size_t from) {
    long long v = 0;
    for (size_t i = from; i < s.size() && s[i] >= L'0' && s[i] <= L'9'; ++i) {
        v = v * 10 + (s[i] - L'0');
        if (v > 1000000000LL) return 1000000000;
    }
    return static_cast<int>(v);
}

int LookupKey(const std::wstring& t) {
    if (t.size() == 1) {
        wchar_t c = t[0];
        if ((c >= L'A' && c <= L'Z') || (c >= L'0' && c <= L'9')) return c;
    }
    if (t.size() >= 2 && t.size() <= 3 && t[0] == L'F') {
        bool allDigits = true;
        for (size_t i = 1; i < t.size(); ++i) {
            if (t[i] < L'0' || t[i] > L'9') allDigits = false;
        }
        if (allDigits) {
            int n = ParseUInt(t, 1);
            if (n >= 1 && n <= 24) return VK_F1 + n - 1;
        }
    }
    for (const auto& k : kNames) {
        if (t == k.name) return k.vk;
    }
    return -1;
}

bool IsSeparator(wchar_t c) {
    return c == L',' || c == L' ' || c == L';' || c == L'\r' || c == L'\n' || c == L'\t';
}

}  // namespace

bool ParseSequence(const std::wstring& text, std::vector<Step>& out, std::wstring& err) {
    std::vector<std::wstring> tokens;
    std::wstring cur;
    for (wchar_t c : text) {
        if (IsSeparator(c)) {
            if (!cur.empty()) { tokens.push_back(cur); cur.clear(); }
        } else {
            cur += static_cast<wchar_t>(std::towupper(c));
        }
    }
    if (!cur.empty()) tokens.push_back(cur);

    for (const auto& tok : tokens) {
        Step s;
        if (tok.rfind(L"WAIT:", 0) == 0) {
            s.waitMs = ParseUInt(tok, 5);
            if (s.waitMs > kMaxWaitMs) s.waitMs = kMaxWaitMs;
            out.push_back(s);
            continue;
        }
        size_t pos = 0;
        while (true) {
            size_t p = tok.find(L'+', pos);
            std::wstring part = tok.substr(pos, p == std::wstring::npos ? std::wstring::npos : p - pos);
            int vk = LookupKey(part);
            if (vk < 0) {
                err = L"ไม่รู้จักปุ่ม: " + tok;
                return false;
            }
            s.keys.push_back(vk);
            if (p == std::wstring::npos) break;
            pos = p + 1;
        }
        out.push_back(s);
    }

    if (out.empty()) {
        err = L"ยังไม่ได้ใส่ปุ่ม";
        return false;
    }
    return true;
}

}  // namespace autokey
