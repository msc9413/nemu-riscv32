#include <am.h>
#include <riscv32.h>
#include <klib.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;

_Context* __am_irq_handle(_Context *c) {
  _Context *next = c;

  if (user_handler) {
    _Event ev = {0};
    switch (c->cause) {
      case -1: // 自陷异常
        ev.event = _EVENT_YIELD;
        break;
      // 系统调用
      case 0: // SYS_exit
      case 1: // SYS_yield
      case 2: // SYS_open
      case 3: // SYS_read
      case 4: // SYS_write
      case 7: // SYS_close
      case 8: // SYS_lseek
      case 9: // SYS_brk
      case 13: // SYS_execve
        ev.event = _EVENT_SYSCALL;
        break;
      default: ev.event = _EVENT_ERROR; break;
    }

    next = user_handler(ev, c);
    if (next == NULL) {
      next = c;
    }
  }

  return next;
}

extern void __am_asm_trap(void);

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  // initialize exception entry
  asm volatile("csrw stvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  return NULL;
}

void _yield() {
  asm volatile("li a7, -1; ecall");
}

int _intr_read() {
  return 0;
}

void _intr_write(int enable) {
}
