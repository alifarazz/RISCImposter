#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./header/memory.h"
#include "./header/utils.h"

#include "./arch/mips/mips.h"

#define CHAR_BUFFER_SIZE 1000
#define INSTRUCTION_LOCATION "../ins.txt"
#define MEMORY_DUMP_LOCATION "../mem_dump.txt"

#define MEM_SIZE_BYTE 100

char g_char_buf[CHAR_BUFFER_SIZE];

void reg_dump(int idx, FILE* fp)
{
  if (fp == NULL || idx > REGFILE_SIZE - 1)
    return;
  puts("[INFO] Reg Dump Start");
  if (idx > -1) {
    fprintf(fp, "$%d: %08x\n", idx, g_regfile[idx]);
    return;
  }
  for (idx = 0; idx < 8 * (int)sizeof(int32_t); idx++)
    fprintf(fp, "$%d: %08x\n", idx, g_regfile[idx]);
  puts("[INFO] Register Dump Done");
}

/* Dumps whole memory if $idx == -1
 * else it dumps mem $idx to fp */
void mem_dump(int idx, FILE* fp)
{
  int32_t foo;

  if (fp == NULL)
    return;
  puts("[INFO] Mmeory Dump Start");
  if (idx > -1) {
    read_memory(idx, &foo);
    fprintf(fp, "$%i: 0x%02x\n", idx, foo);
    return;
  }
  for (idx = 0; idx < MEM_SIZE_BYTE; idx++) {
    fprintf(fp, "$%i: 0x%02x\n", idx, g_mainmem[idx]);
  }
  puts("[INFO] Memory Dump Done");
}

/* for parsing input data.
 * it takes a 32 byte char array  of '1' and '0' and converts them
 * into an int32_t. it assmues 32 bytes of memory, iterating over them */
static int32_t convert_str_bin(const char* buf)
{
  int i;
  int32_t res = 0x00000000;

  for (i = 0; i < 8 * (int)sizeof(int32_t); i++) {
    res <<= 1;	 /* add zero to right */
    if (buf[i] == '1') /* if it's a 1 then */
      res |= 1;	/* toggle the LSB bit */
  }
  return res;
}

int main()
{
  int i;
  FILE *insfp, *memfp;

  /* TODO: should be in init_cpu or boot_cpu */
  if (NULL == (insfp = imposter_open_file(INSTRUCTION_LOCATION, "r")))
    goto exit_clean;

  g_instructions[0] = 0x00000000;
  g_instructions[1] = 0x00000000;
  g_instructions[2] = 0x00000000;
  g_instructions[3] = 0x00000000;
  g_instructions[4] = 0x00000000;
  for (i = 5; NULL != fgets(g_char_buf, CHAR_BUFFER_SIZE, insfp);) {
    if (g_char_buf[0] == '#') /* it's a comment */
      continue;

    /* assume anything after the instruction is a comment */
    g_char_buf[INSTRUCTION_SIZE_BYTE * 8] = '\0';

    g_instructions[i++] = convert_str_bin(g_char_buf);
  }
  fclose(insfp);
  g_instructions[i++] = 0x00000000;
  g_instructions[i++] = 0x00000000;
  g_instructions[i++] = 0x00000000;
  g_instructions[i++] = 0x00000000;
  g_instructions[i++] = 0x00000000;

  cpu_init(MAX_MEM_SIZE_BYTE, i);
  g_regfile[1] = 2;
  g_regfile[2] = 4;
  /* main cpu loop :) */
  while (cpu_tick()) cpu_tock();

  puts("\n");
  reg_dump(-1, stdout);
  if (NULL != (memfp = imposter_open_file(MEMORY_DUMP_LOCATION, "w")))
    mem_dump(-1, memfp);

  /* if ((fp = fopen("../regfile.txt", "w")) == NULL) { */
  /*   perror("fopen() failed regfile."); */
  /*   goto exit_clean; */
  /* } */
  /* lol_reg_dump(-1, context, fp); */
  /* fclose(fp); */
  /* if ((fp = fopen("../mainmem.txt", "w")) == NULL) { */
  /*   perror("fopen() failed main memory"); */
  /*   goto exit_clean; */
  /* } */
  /* lol_mem_dump(-1, context, fp); */
  /* fclose(fp); */

  return EXIT_SUCCESS;

exit_clean:
  return EXIT_FAILURE;
}
