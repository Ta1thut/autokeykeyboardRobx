#include "main_window.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <memory>

#include "key_parser.h"
#include "version.h"

#ifndef MOD_NOREPEAT
#define MOD_NOREPEAT 0x4000
#endif

namespace autokey {
namespace {

enum ControlId {
    IDC_KEYS = 101, IDC_HOLD, IDC_GAP, IDC_MIN, IDC_SEC, IDC_REPEAT,
    IDC_DELAY, IDC_JITTER, IDC_NOW, IDC_START, IDC_TEST, IDC_STATUS
};

constexpr int kHotkeyId = 1;        // F8
constexpr int kIconResourceId = 1;  // ดู resources/app.rc
constexpr int kClientWidth = 422;   // หน่วยที่ 96 DPI
constexpr int kClientHeight = 394;
constexpr wchar_t kClassName[] = L"AutoKeyMainWindow";

const int kLockedWhileRunning[] = {
    IDC_KEYS, IDC_HOLD, IDC_GAP, IDC_MIN, IDC_SEC,
    IDC_REPEAT, IDC_DELAY, IDC_JITTER, IDC_NOW, IDC_TEST
};

BOOL CALLBACK ApplyFont(HWND child, LPARAM font) {
    SendMessageW(child, WM_SETFONT, static_cast<WPARAM>(font), TRUE);
    return TRUE;
}

int QueryScreenDpi() {
    HDC dc = GetDC(nullptr);
    int dpi = dc ? GetDeviceCaps(dc, LOGPIXELSX) : 96;
    if (dc) ReleaseDC(nullptr, dc);
    return dpi > 0 ? dpi : 96;
}

HICON LoadAppIcon(HINSTANCE instance, int cx, int cy) {
    HANDLE h = LoadImageW(instance, MAKEINTRESOURCEW(kIconResourceId), IMAGE_ICON, cx, cy, 0);
    return h ? static_cast<HICON>(h) : LoadIconW(nullptr, IDI_APPLICATION);
}

}  // namespace

// ------------------------------------------------------------------ entry
int MainWindow::Run(HINSTANCE instance, int showCmd) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    MainWindow window;
    window.dpi_ = QueryScreenDpi();

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProcThunk;
    wc.hInstance = instance;
    wc.hIcon = LoadAppIcon(instance, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON));
    wc.hIconSm = LoadAppIcon(instance, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON));
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(static_cast<INT_PTR>(COLOR_BTNFACE + 1));
    wc.lpszClassName = kClassName;
    if (!RegisterClassExW(&wc)) return 1;

    RECT r = {0, 0, window.Px(kClientWidth), window.Px(kClientHeight)};
    const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRect(&r, style, FALSE);

    const std::wstring title = L"AutoKey v" AUTOKEY_VERSION_WSTR L" zZ";
    HWND h = CreateWindowExW(WS_EX_TOPMOST, kClassName, title.c_str(), style,
                             CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
                             nullptr, nullptr, instance, &window);
    if (!h) return 1;
    ShowWindow(h, showCmd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        if (!IsDialogMessageW(h, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    return static_cast<int>(msg.wParam);
}

// ------------------------------------------------------------------ window proc
LRESULT CALLBACK MainWindow::WndProcThunk(HWND h, UINT m, WPARAM w, LPARAM l) {
    MainWindow* self = nullptr;
    if (m == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(l);
        self = static_cast<MainWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(h, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        if (self) self->hwnd_ = h;
    } else {
        self = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(h, GWLP_USERDATA));
    }
    return self ? self->HandleMessage(h, m, w, l) : DefWindowProcW(h, m, w, l);
}

LRESULT MainWindow::HandleMessage(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
    case WM_CREATE:
        CreateControls();
        // MOD_NOREPEAT: ถือค้างปุ่ม F8 แล้วไม่ยิงซ้ำจนสลับเริ่ม/หยุดรัว
        if (!RegisterHotKey(h, kHotkeyId, MOD_NOREPEAT, VK_F8))
            SetStatus(L"Fail to press F8 (May have another program runing) Try again!");
        return 0;

    case WM_COMMAND:
        if (HIWORD(w) == BN_CLICKED) {
            if (LOWORD(w) == IDC_START) Toggle(false);
            else if (LOWORD(w) == IDC_TEST) Toggle(true);
        }
        return 0;

    case WM_HOTKEY:
        if (w == kHotkeyId) Toggle(false);
        return 0;

    case MacroRunner::kMsgStatus: {
        std::unique_ptr<std::wstring> text(reinterpret_cast<std::wstring*>(l));
        if (runner_.Busy()) SetStatus(*text);
        return 0;
    }

    case MacroRunner::kMsgDone:
        runner_.ClearBusy();
        SetRunningUI(false);
        SetStatus(L"Stopped  (Ready to start again)");
        return 0;

    case WM_DESTROY:
        runner_.Stop();
        UnregisterHotKey(h, kHotkeyId);
        if (font_) { DeleteObject(font_); font_ = nullptr; }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(h, m, w, l);
}

// ------------------------------------------------------------------ UI
void MainWindow::CreateControls() {
    HWND h = hwnd_;
    auto child = [this](DWORD exStyle, const wchar_t* cls, const wchar_t* text, DWORD style,
                        int x, int y, int w, int hh, int id) {
        return CreateWindowExW(exStyle, cls, text, WS_CHILD | WS_VISIBLE | style,
                               Px(x), Px(y), Px(w), Px(hh), hwnd_,
                               reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);
    };
    auto label = [&](const wchar_t* t, int x, int y, int w, int hh = 20) {
        child(0, L"STATIC", t, 0, x, y, w, hh, 0);
    };
    auto edit = [&](int id, const wchar_t* t, int x, int y, int w, bool numberOnly = true) {
        HWND e = child(WS_EX_CLIENTEDGE, L"EDIT", t,
                       WS_TABSTOP | ES_AUTOHSCROLL | (numberOnly ? ES_NUMBER : 0), x, y, w, 24, id);
        SendMessageW(e, EM_LIMITTEXT, numberOnly ? 7 : 1000, 0);
    };
    auto check = [&](int id, const wchar_t* t, int x, int y, int w) {
        child(0, L"BUTTON", t, WS_TABSTOP | BS_AUTOCHECKBOX, x, y, w, 22, id);
    };
    auto button = [&](int id, const wchar_t* t, int x, int y, int w, int hh, bool isDefault) {
        child(0, L"BUTTON", t, WS_TABSTOP | (isDefault ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON), x, y, w, hh, id);
    };

    label(L"Keys to press (separated by commas or spaces)", 16, 12, 380);
    edit(IDC_KEYS, L"W,A,S,D", 16, 34, 380, false);
    // label(L"ปุ่มที่ใช้ได้: A-Z  0-9  F1-F12  SPACE ENTER TAB ESC SHIFT CTRL ALT\n"
    //       L"UP DOWN LEFT RIGHT  LMB RMB (คลิกเมาส์)\n"
    //       L"กดพร้อมกัน: W+A    หน่วงเวลาเอง: WAIT:500 (ms)",
    //       16, 64, 390, 52);

    label(L"Key hold duration (ms)", 16, 126, 140);
    edit(IDC_HOLD, L"50", 160, 123, 60);
    label(L"Delay between keys (ms)", 235, 126, 110);
    edit(IDC_GAP, L"100", 345, 123, 60);

    label(L"Repeat every", 16, 164, 60);
    edit(IDC_MIN, L"5", 80, 161, 50);
    label(L"Min", 136, 164, 40);
    edit(IDC_SEC, L"0", 185, 161, 50);
    label(L"Sec", 241, 164, 50);

    label(L"Number of cycles (0 = Loop)", 16, 202, 170);
    edit(IDC_REPEAT, L"0", 190, 199, 50);
    label(L"Initial delay (sec)", 255, 202, 110);
    edit(IDC_DELAY, L"3", 370, 199, 36);

    check(IDC_JITTER, L"Randomize timing ±20% ", 16, 236, 260);
    check(IDC_NOW, L"Tap once when start", 16, 260, 260);
    CheckDlgButton(h, IDC_NOW, BST_CHECKED);

    button(IDC_START, L"Start (F8)", 16, 296, 240, 40, true);
    button(IDC_TEST, L"Test once", 266, 296, 140, 40, false);

    child(WS_EX_STATICEDGE, L"STATIC", L"Ready (F8 = Start/Stop, works even while in-game)",
          SS_CENTERIMAGE, 16, 348, 390, 30, IDC_STATUS);

    font_ = CreateFontW(-Px(16), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0,
                        CLEARTYPE_QUALITY, 0, L"Segoe UI");
    EnumChildWindows(h, ApplyFont, reinterpret_cast<LPARAM>(font_));
}

void MainWindow::SetStatus(const std::wstring& text) {
    SetDlgItemTextW(hwnd_, IDC_STATUS, text.c_str());
}

void MainWindow::SetRunningUI(bool running) {
    for (int id : kLockedWhileRunning) EnableWindow(GetDlgItem(hwnd_, id), !running);
    SetDlgItemTextW(hwnd_, IDC_START, running ? L"Stop (F8)" : L"Start (F8)");
}

int MainWindow::GetInt(int id, int fallback, int lo, int hi) const {
    BOOL ok = FALSE;
    UINT v = GetDlgItemInt(hwnd_, id, &ok, FALSE);
    if (!ok) return fallback;
    int n = static_cast<int>((std::min)(v, static_cast<UINT>(hi)));
    return (std::max)(n, lo);
}

void MainWindow::Toggle(bool once) {
    if (runner_.Busy()) {                       // กำลังทำงาน -> สั่งหยุด
        if (runner_.Running()) {
            runner_.Stop();
            SetStatus(L"Stopping...");
        }
        return;
    }

    wchar_t keys[1024];
    GetDlgItemTextW(hwnd_, IDC_KEYS, keys, 1024);

    MacroConfig cfg;
    std::wstring err;
    if (!ParseSequence(keys, cfg.steps, err)) {
        MessageBoxW(hwnd_, err.c_str(), L"AutoKey", MB_ICONWARNING);
        return;
    }

    cfg.holdMs      = GetInt(IDC_HOLD, 50, 0, 60000);
    cfg.gapMs       = GetInt(IDC_GAP, 100, 0, 60000);
    cfg.intervalSec = GetInt(IDC_MIN, 5, 0, 10000) * 60 + GetInt(IDC_SEC, 0, 0, 86400);
    if (cfg.intervalSec < 1) cfg.intervalSec = 1;
    cfg.repeat      = GetInt(IDC_REPEAT, 0, 0, 1000000);
    cfg.startDelay  = GetInt(IDC_DELAY, 3, 0, 3600);
    cfg.jitter      = IsDlgButtonChecked(hwnd_, IDC_JITTER) == BST_CHECKED;
    cfg.runNow      = IsDlgButtonChecked(hwnd_, IDC_NOW) == BST_CHECKED;
    cfg.once        = once;

    if (runner_.Start(hwnd_, cfg)) SetRunningUI(true);
}

}  // namespace autokey
