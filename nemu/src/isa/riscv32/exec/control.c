#include "cpu/exec.h"

make_EHelper(jal){
    s0 = cpu.pc + 4;
    rtl_sr(id_dest->reg, &s0, 4);
    rtl_add(&decinfo.jmp_pc, &cpu.pc, &id_src->val);
    rtl_j(decinfo.jmp_pc); 
    
    print_asm_template2(jal);
}

make_EHelper(jalr){
    s0 = cpu.pc + 4;
    rtl_sr(id_dest->reg, &s0, 4);
    rtl_add(&decinfo.jmp_pc, &id_src->val, &id_src2->val);
    rtl_j(decinfo.jmp_pc);

    print_asm_template2(jalr);
}

static uint32_t branch_table[8] = {
    RELOP_EQ, RELOP_NE, RELOP_FALSE, RELOP_FALSE, RELOP_LT, RELOP_GE, RELOP_LTU, RELOP_GEU
};

make_EHelper(branch) {
    rtl_jrelop(branch_table[decinfo.isa.instr.funct3], &id_src->val, &id_src2->val, decinfo.jmp_pc);
    switch (branch_table[decinfo.isa.instr.funct3]) {
        case RELOP_EQ: print_asm_template3(beq); break;
        case RELOP_NE: print_asm_template3(bne); break;
        case RELOP_LT: print_asm_template3(blt); break;
        case RELOP_GE: print_asm_template3(bge); break;
        case RELOP_LTU: print_asm_template3(bltu); break;
        case RELOP_GEU: print_asm_template3(bgeu); break;
        default: assert(!"Unfind the branch opcode"); break;
    }
}