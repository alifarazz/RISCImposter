#ifndef LOL_ARCH_MIPS_MEMORY_H
#define LOL_ARCH_MIPS_MEMORY_H

#include <stdint.h>

#define MAX_INSTRUCTION_COUNT 1000
#define MEM_SIZE_BYTE 1000
#define REGFILE_SIZE 32

#define MAX_INSTRUCTION_4_BYTE MAX_INSTRUCTION_COUNT


int32_t g_regfile[REGFILE_SIZE];
int32_t g_instructions[MAX_INSTRUCTION_COUNT];
int8_t *g_mainmem;

int32_t g_instructions_len;


#endif
