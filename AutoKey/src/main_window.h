#ifndef AUTOKEY_MAIN_WINDOW_H
#define AUTOKEY_MAIN_WINDOW_H

#include "win_common.h"

#include <string>

#include "macro_runner.h"

namespace autokey {

class MainWindow {
public:
    // สร้างหน้าต่างและรัน message loop จนกว่าจะปิดโปรแกรม
    static int Run(HINSTANCE instance, int showCmd);

private:
    static LRESULT CALLBACK WndProcThunk(HWND h, UINT m, WPARAM w, LPARAM l);
    LRESULT HandleMessage(HWND h, UINT m, WPARAM w, LPARAM l);

    void CreateControls();
    void Toggle(bool once);
    void SetStatus(const std::wstring& text);
    void SetRunningUI(bool running);
    int  GetInt(int id, int fallback, int lo, int hi) const;
    int  Px(int v) const { return MulDiv(v, dpi_, 96); }   // ปรับขนาดตาม DPI ของจอ

    HWND hwnd_ = nullptr;
    HFONT font_ = nullptr;
    int dpi_ = 96;
    MacroRunner runner_;
};

}  // namespace autokey

#endif  // AUTOKEY_MAIN_WINDOW_H
