#include "cpu/exec.h"

make_EHelper(ld) {
  rtl_lm(&s0, &id_src->addr, decinfo.width);
  switch (decinfo.width) {
    case 4: print_asm_template2(lw); break; // lw
    case 2:      // lhu和lh的区别就在于funct3的第三位
      if (decinfo.isa.instr.funct3 >> 2) {  // lhu
        print_asm_template2(lhu);
      } else {                              // lh
        rtl_sext(&s0, &s0, 2);
        print_asm_template2(lh);            
      }
      break;
    case 1:       // lbu和lb的区别就在于funct3的第三位
      if (decinfo.isa.instr.funct3 >> 2) {  // lbu
        print_asm_template2(lbu);   
      } else {                              // lb
        rtl_sext(&s0, &s0, 1);
        print_asm_template2(lb);
      }
      break;
    default: assert(0);
  }
  rtl_sr(id_dest->reg, &s0, 4);
}

make_EHelper(st) {
  rtl_sm(&id_src->addr, &id_dest->val, decinfo.width);

  switch (decinfo.width) {
    case 4: print_asm_template2(sw); break;
    case 2: print_asm_template2(sh); break;
    case 1: print_asm_template2(sb); break;
    default: assert(0);
  }
}
