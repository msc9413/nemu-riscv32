#include <am.h>
#include <amdev.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

size_t __am_input_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_INPUT_KBD: {
      _DEV_INPUT_KBD_t *kbd = (_DEV_INPUT_KBD_t *)buf;
      uint32_t keyboard_code = inl(KBD_ADDR);  // 从MMIO中获取键盘码
      kbd->keydown = keyboard_code & KEYDOWN_MASK ? 1 : 0;
      kbd->keycode = keyboard_code & ~KEYDOWN_MASK;     // _KEY_NONE == 0
      return sizeof(_DEV_INPUT_KBD_t);
    }
  }
  return 0;
}
