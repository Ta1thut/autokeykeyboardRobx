#ifndef AUTOKEY_KEY_PARSER_H
#define AUTOKEY_KEY_PARSER_H

#include <string>
#include <vector>

namespace autokey {

// หนึ่งขั้นตอนในลำดับการกด
//  - keys   : ปุ่มที่กดพร้อมกัน (เช่น CTRL+C -> {VK_CONTROL, 'C'})
//  - waitMs : ถ้า >= 0 หมายถึงขั้นตอน "รอ" (WAIT:500) ไม่ได้กดปุ่ม
struct Step {
    std::vector<int> keys;
    int waitMs = -1;
};

// แปลงข้อความ เช่น "W,A,S,D, CTRL+C, WAIT:500" เป็นลำดับ Step
// คืน false พร้อมข้อความใน err เมื่อมีปุ่มที่ไม่รู้จัก
bool ParseSequence(const std::wstring& text, std::vector<Step>& out, std::wstring& err);

}  // namespace autokey

#endif  // AUTOKEY_KEY_PARSER_H
