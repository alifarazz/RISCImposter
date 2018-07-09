#ifndef IMPOSTER_ARCH_MIPS_MIPS_H
#define IMPOSTER_ARCH_MIPS_MIPS_H

#include <assert.h>
#include <memory.h>

#include "../../header/utils.h"
#include "./compnents.h"
#include "./defs.h"
#include "./memory.h"
#include "./pipeline.h"
#include "./stages.h"

int32_t g_instruction_count;

/* set all memory to a specific value */
void set_memory(int32_t c)
{
  for (int32_t i = 0; i < g_mainmemsize / 4; i++) write_memory(i, c);
}

/* cpu initialization */
void cpu_init(const int32_t mainmem_size_byte, const uint32_t instruction_count)
{
  /* start at the begining of instructions */
  prgc = 5;

  /* initialize pipeline registers */
  pipelineIFIDn.inst = 0;
  pipelineIFIDn.pc   = 5;

  pipelineIDEXn = (struct PipelineIDEX){0, 0, 0, 0, 0, {0}, {0}, {0}};

  pipelineEXMEMn = (struct PipelineEXMEM){0, 0, 0, {0}, {0}};

  pipelineMEMWBn = (struct PipelineMEMWB){0, 0, 0, {0}};

  pipelineIFIDp  = pipelineIFIDn;
  pipelineIDEXp  = pipelineIDEXn;
  pipelineEXMEMp = pipelineEXMEMn;
  pipelineMEMWBp = pipelineMEMWBn;

  /* initialize regfile */
  assert(REGFILE_SIZE > 0);
  memset(g_regfile, 0, REGFILE_SIZE * sizeof(g_regfile[0]));

  /* initialize memory */
  assert(MAX_MEM_SIZE_BYTE >= mainmem_size_byte);
  g_mainmem     = imposter_malloc(mainmem_size_byte, 1);
  g_mainmemsize = mainmem_size_byte;
  set_memory(0);

  g_instruction_count = instruction_count;
}

/* cpu terminate */
void cpu_term() {}

/* cpu on rising edge of clock */
/* the fucntion will only return 0 if prgc reaches g_instruction_count */
int cpu_tick()
{
  /* check if pc has reached end of instructions */
  if (g_instruction_count <= prgc) {
    return 0;
  }
  /* write-back should happen before decode because regfile access
   * fetch should happen before decode because pipeline stalls */
  exec_stage_wb();
  exec_stage_fetch();
  exec_stage_decode();
  exec_stage_exec();
  exec_stage_mem();
  return 1;
}

/* cpu on falling edge of clock */
void cpu_tock()
{
  pipelineEXMEMp = pipelineEXMEMn;
  pipelineIDEXp  = pipelineIDEXn;
  pipelineIFIDp  = pipelineIFIDn;
  pipelineMEMWBp = pipelineMEMWBn;
}

#endif
