#ifndef IMPOSTER_ARCH_MIPS_DEFS_H
#define IMPOSTER_ARCH_MIPS_DEFS_H

#include <stdint.h>

#define INSTRUCTION_SIZE_BYTE 4

#define OPCODE_MASK 0x0000003F
#define FUNC_MASK 0x0000003F
#define IMM_MASK 0x0000FFFF
#define J_ADDR_MASK 0x03FFFFFFF
#define SHAMT_MASK 0x0000003F
#define REG_MASK 0x0000001F

#define OPCODE_J 0x00000002
#define OPCODE_LW 0x00000023
#define OPCODE_SW 0x0000002B
#define OPCODE_BNE 0x00000004
#define OPCODE_BEQ 0x00000005
#define OPCODE_RTYPE 0x00000000
#define FUNC_ADD 0x00000020
#define FUNC_AND 0x00000024
#define FUNC_SUB 0x00000022
#define FUNC_JR 0x00000008
#define FUNC_NOR 0x00000027
#define FUNC_OR 0x00000025
#define FUNC_XOR 0x00000026
#define FUNC_SLT 0x0000002A

/* return opcode zerofilled to 32 bits */
int32_t get_opcode(int32_t inst) { return (inst >> 26) & OPCODE_MASK; }

int16_t get_imm(int32_t inst) { return inst & IMM_MASK; }

int32_t get_rs(int32_t inst) { return (inst >> 21) & REG_MASK; }

int32_t get_rt(int32_t inst) { return (inst >> 16) & REG_MASK; }

int32_t get_rd(int32_t inst) { return (inst >> 11) & REG_MASK; }

int32_t get_shamt(int32_t inst) { return (inst >> 6) & SHAMT_MASK; }

int32_t get_func(int32_t inst) { return inst & FUNC_MASK; }

int32_t get_address_j(int32_t inst) { return inst ^ (inst & (~J_ADDR_MASK)); }

char is_jtype(int32_t inst) { return get_opcode(inst) == OPCODE_J; }

char is_rtype(int32_t inst) { return get_opcode(inst) == OPCODE_RTYPE; }

#endif
