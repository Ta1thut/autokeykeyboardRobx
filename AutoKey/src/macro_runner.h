#ifndef AUTOKEY_MACRO_RUNNER_H
#define AUTOKEY_MACRO_RUNNER_H

#include "win_common.h"

#include <atomic>
#include <string>
#include <vector>

#include "key_parser.h"

namespace autokey {

struct MacroConfig {
    std::vector<Step> steps;
    int  holdMs      = 50;    // กดค้างต่อปุ่ม
    int  gapMs       = 100;   // เว้นระหว่างปุ่ม
    int  intervalSec = 300;   // ทำซ้ำทุกกี่วินาที (นับจาก "เริ่ม" รอบก่อนหน้า)
    int  repeat      = 0;     // จำนวนรอบ (0 = ไม่จำกัด)
    int  startDelay  = 3;     // หน่วงก่อนเริ่ม (วินาที)
    bool jitter      = false; // สุ่มเวลา ±20%
    bool runNow      = true;  // กดรอบแรกทันที
    bool once        = false; // ทดสอบ 1 รอบ
};

// รันมาโครบน worker thread แยกจาก UI
// สื่อสารกลับผ่านข้อความ Windows:
//   kMsgStatus : lParam = std::wstring* (ผู้รับต้อง delete)
//   kMsgDone   : ส่งเมื่อ thread จบการทำงาน
class MacroRunner {
public:
    static constexpr UINT kMsgStatus = WM_APP + 1;
    static constexpr UINT kMsgDone   = WM_APP + 2;

    MacroRunner() = default;
    ~MacroRunner();                                  // หยุด + รอ thread จบ (ปล่อยปุ่มที่ค้างให้ครบ)
    MacroRunner(const MacroRunner&) = delete;
    MacroRunner& operator=(const MacroRunner&) = delete;

    bool Start(HWND target, const MacroConfig& cfg);
    void Stop() { run_ = false; }

    bool Busy() const    { return busy_; }
    bool Running() const { return run_; }
    void ClearBusy()     { busy_ = false; }

private:
    static DWORD WINAPI ThreadProc(LPVOID param);
    void Execute(HWND target, const MacroConfig& cfg);
    void RunSequence(const MacroConfig& cfg);
    void SleepMs(int ms);

    std::atomic<bool> run_{false};
    std::atomic<bool> busy_{false};
    HANDLE thread_ = nullptr;
};

}  // namespace autokey

#endif  // AUTOKEY_MACRO_RUNNER_H
