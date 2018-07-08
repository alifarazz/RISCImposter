#ifndef IMPOSTER_ARCH_MIPS_MEMORY_H
#define IMPOSTER_ARCH_MIPS_MEMORY_H

#include <stdlib.h>

#define MAX_INSTRUCTION_COUNT 1000
#define MEM_SIZE_BYTE 1000
#define REGFILE_SIZE 32

#define MAX_INSTRUCTION_4_BYTE MAX_INSTRUCTION_COUNT

union ConverterIntChar {
  int32_t i;
  int8_t c[4];
};

int32_t g_regfile[REGFILE_SIZE];
int32_t g_instructions[MAX_INSTRUCTION_COUNT];
int8_t* g_mainmem;
int32_t g_mainmemsize;

int32_t prgc;

int32_t g_instructions_len;

int read_memory(const int32_t idx, int32_t* content)
{
  union ConverterIntChar convert_ic;
  if (g_mainmem == NULL)
    return -1; /* invalid context */
  if (content == NULL)
    return -2; /* invalid content */
  if (idx + 3 >= g_mainmemsize)
    return 1; /* out of memeory */

    /* Read 4 bytes from main memory and convert it to a 4 byte word */
#ifdef __BIG_ENDIAN__
  convert_ic.c[0] = g_mainmem[idx];
  convert_ic.c[1] = g_mainmem[idx + 1];
  convert_ic.c[2] = g_mainmem[idx + 2];
  convert_ic.c[3] = g_mainmem[idx + 3];
#else
  convert_ic.c[0]    = g_mainmem[idx + 3];
  convert_ic.c[1]    = g_mainmem[idx + 2];
  convert_ic.c[2]    = g_mainmem[idx + 1];
  convert_ic.c[3]    = g_mainmem[idx];
#endif
  *content = convert_ic.i;
  return 0;
}

int write_memory(int32_t idx, int32_t content)
{
  union ConverterIntChar convert_ic;
  if (g_mainmem == NULL)
    return -1; /* invalid context */
  if (idx + 3 >= g_mainmemsize)
    return 1; /* out of memory */

  /* Write 'content' to 4 consecutive byte at main memory */
  convert_ic.i = content;
#ifdef __BIG_ENDIAN__
  g_mainmem[idx]     = convert_ic.c[0];
  g_mainmem[idx + 1] = convert_ic.c[1];
  g_mainmem[idx + 2] = convert_ic.c[2];
  g_mainmem[idx + 3] = convert_ic.c[3];
#else
  g_mainmem[idx + 3] = convert_ic.c[0];
  g_mainmem[idx + 2] = convert_ic.c[1];
  g_mainmem[idx + 1] = convert_ic.c[2];
  g_mainmem[idx]     = convert_ic.c[3];
#endif
  return 0;
}

#endif
