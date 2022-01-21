#include "rtl/rtl.h"
#include "cpu/decode.h"

extern DecodeInfo decinfo;

void raise_intr(uint32_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  decinfo.isa.sepc = epc;               // 将当前PC值保存到sepc寄存器
  decinfo.isa.scause = NO;              // 在scause寄存器中设置异常号
  decinfo.jmp_pc = decinfo.isa.stvec;   // 从stvec寄存器中取出异常入口地址
  rtl_j(decinfo.jmp_pc);                // 跳转到异常入口地址
}

bool isa_query_intr(void) {
  return false;
}
