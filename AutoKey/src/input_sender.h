#ifndef AUTOKEY_INPUT_SENDER_H
#define AUTOKEY_INPUT_SENDER_H


namespace autokey {

// ส่งเหตุการณ์ปุ่มคีย์บอร์ด/เมาส์ไปยังระบบ (ใช้ scan code เพื่อให้เกมรับได้)
// vk = Virtual-Key code, down = true กดลง / false ปล่อย
void SendKey(int vk, bool down);

}  // namespace autokey

#endif  // AUTOKEY_INPUT_SENDER_H
