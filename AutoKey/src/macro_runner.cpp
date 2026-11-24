#include "macro_runner.h"

#include <cstdlib>

#include "input_sender.h"

namespace autokey {
namespace {

struct ThreadCtx {
    MacroRunner* self;
    HWND target;
    MacroConfig cfg;
};

int Jitter(int ms, bool enabled) {
    if (!enabled || ms < 5) return ms;
    int span = ms / 5;  // ±20%
    return ms + (std::rand() % (2 * span + 1)) - span;
}

std::wstring Num(long long v) {
    wchar_t buf[32];
    wsprintfW(buf, L"%d", static_cast<int>(v));
    return buf;
}

std::wstring FormatTime(long long sec) {
    wchar_t buf[48];
    if (sec >= 3600)
        wsprintfW(buf, L"%d:%02d:%02d", static_cast<int>(sec / 3600),
                  static_cast<int>((sec % 3600) / 60), static_cast<int>(sec % 60));
    else
        wsprintfW(buf, L"%02d:%02d", static_cast<int>(sec / 60), static_cast<int>(sec % 60));
    return buf;
}

void PostStatus(HWND target, const std::wstring& text) {
    auto* msg = new std::wstring(text);
    if (!PostMessageW(target, MacroRunner::kMsgStatus, 0, reinterpret_cast<LPARAM>(msg)))
        delete msg;  // หน้าต่างปิดไปแล้ว กัน memory leak
}

}  // namespace

MacroRunner::~MacroRunner() {
    Stop();
    if (thread_) {
        WaitForSingleObject(thread_, 3000);
        CloseHandle(thread_);
        thread_ = nullptr;
    }
}

bool MacroRunner::Start(HWND target, const MacroConfig& cfg) {
    if (busy_) return false;

    // thread รอบก่อนส่ง kMsgDone แล้ว (busy_ ถูกเคลียร์) ปิด handle เก่าได้เลย
    if (thread_) {
        CloseHandle(thread_);
        thread_ = nullptr;
    }

    run_ = true;
    busy_ = true;

    auto* ctx = new ThreadCtx{this, target, cfg};
    thread_ = CreateThread(nullptr, 0, ThreadProc, ctx, 0, nullptr);
    if (!thread_) {
        run_ = false;
        busy_ = false;
        delete ctx;
        return false;
    }
    return true;
}

DWORD WINAPI MacroRunner::ThreadProc(LPVOID param) {
    auto* ctx = static_cast<ThreadCtx*>(param);
    ctx->self->Execute(ctx->target, ctx->cfg);
    delete ctx;
    return 0;
}

// หลับแบบหยุดได้ทันทีเมื่อสั่ง Stop
void MacroRunner::SleepMs(int ms) {
    ULONGLONG end = GetTickCount64() + static_cast<ULONGLONG>(ms < 0 ? 0 : ms);
    while (run_ && GetTickCount64() < end) Sleep(10);
}

void MacroRunner::RunSequence(const MacroConfig& cfg) {
    for (const auto& step : cfg.steps) {
        if (!run_) return;

        if (step.waitMs >= 0) {
            SleepMs(Jitter(step.waitMs, cfg.jitter));
            continue;
        }
        // กดลงแล้ว "ต้องปล่อยเสมอ" แม้ถูกสั่ง Stop กลางคัน (SleepMs จะคืนค่าทันที)
        for (int k : step.keys) SendKey(k, true);
        SleepMs(Jitter(cfg.holdMs, cfg.jitter));
        for (size_t i = step.keys.size(); i-- > 0;) SendKey(step.keys[i], false);
        SleepMs(Jitter(cfg.gapMs, cfg.jitter));
    }
}

void MacroRunner::Execute(HWND target, const MacroConfig& cfg) {
    // นับถอยหลังก่อนเริ่ม ให้เวลาสลับไปหน้าต่างเกม
    for (int i = cfg.startDelay; i > 0 && run_; --i) {
        PostStatus(target, L"Start in " + Num(i) + L" sec... tab your game");
        SleepMs(1000);
    }

    if (cfg.once) {
        if (run_) {
            PostStatus(target, L"Pressing (Test once)...");
            RunSequence(cfg);
        }
    } else {
        const ULONGLONG intervalMs = static_cast<ULONGLONG>(cfg.intervalSec) * 1000;
        int round = 0;
        bool first = true;
        ULONGLONG roundStart = GetTickCount64();

        while (run_) {
            // รอบแรก: ถ้าติ๊ก "กดทันที" ก็ข้ามการรอ
            // รอบถัดไป: นับจากเวลาที่ "เริ่ม" รอบก่อน เพื่อไม่ให้เวลาเลื่อนสะสม
            if (!(first && cfg.runNow)) {
                ULONGLONG deadline = (first ? GetTickCount64() : roundStart) + intervalMs;
                long long lastShown = -1;
                while (run_ && GetTickCount64() < deadline) {
                    long long left = static_cast<long long>((deadline - GetTickCount64() + 999) / 1000);
                    if (left != lastShown) {
                        lastShown = left;
                        PostStatus(target, L"Round " + Num(round + 1) + L" Start in " + FormatTime(left) +
                                               L"   (Finished " + Num(round) + L" Times)");
                    }
                    Sleep(50);
                }
            }
            first = false;
            if (!run_) break;

            roundStart = GetTickCount64();
            PostStatus(target, L"Pressing... (Round " + Num(round + 1) + L")");
            RunSequence(cfg);
            ++round;
            if (cfg.repeat > 0 && round >= cfg.repeat) break;
        }
    }

    run_ = false;
    PostMessageW(target, kMsgDone, 0, 0);
}

}  // namespace autokey
