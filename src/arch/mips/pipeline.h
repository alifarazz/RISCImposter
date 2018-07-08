#ifndef IMPOSTER_ARCH_MIPS_PIPELINES_H
#define IMPOSTER_ARCH_MIPS_PIPELINES_H

#include <stdint.h>

union ExecuteStageSignals {
  int32_t raw;
  struct {
    char alu_p0_src;
    char alu_p1_src;
    char alu_op;
    char reg_dst;
  } data;
};

union MemoryStageSignals {
  int16_t raw;
  struct {
    char MemWrite;
    char MemRead;
  } data;
};

union WriteBackStageSignals {
  int16_t raw;

  /*          MemtoReg:     0          1        */
  /* RegWrite                                   */
  /*    0               bypass mem   read mem   */
  /*    1                write mem   undefined  */
  struct {
    char MemtoReg;
    char RegWrite;
  } data;
};

struct PipelineIFID {
  /* Program counter */
  int32_t pc;
  /* Fetched instruction */
  int32_t inst;
};

struct PipelineIDEX {
  /* alu input data */
  int32_t reg_read_0, reg_read_1;

  /* immidiate with sign extend */
  int32_t imm_sx;

  /* alu operation is set in SIGEX.alu_op */
  /* int32_t alu_op; */

  /* write-back register */
  int32_t reg_write_dest, reg_write_trgt;

  /* signals */
  union ExecuteStageSignals SIGEX;
  union MemoryStageSignals SIGMEM;
  union WriteBackStageSignals SIGWB;
};

struct PipelineEXMEM {
  /* result of alu arithmatic operation on program counter */
  int32_t alu_result;

  /* the output of reg_read_1 coming out of regfile  */
  int32_t reg_read_1;

  /* write-back register, selected from rd & rt */
  int32_t reg_write;

  /* signals */
  union WriteBackStageSignals SIGWB;
  union MemoryStageSignals SIGMEM;
};

struct PipelineMEMWB {
  /* the result of alu operation in execute stage */
  int32_t alu_result;
  /* the value read from memory in memory stage */
  int32_t mem_read;

  /* the register into which the value is written */
  int32_t reg_write;

  /* signals */
  union WriteBackStageSignals SIGWB;
};

#endif
