#ifndef LOL_ARCH_MIPS_PIPELINES_H
#define LOL_ARCH_MIPS_PIPELINES_H

#include <stdint.h>


union MemoryStageSignals {
    int32_t raw;
    struct {
      char branch; /* TODO: better name? */
      char MemWrite;
      char MemRead;
    } data;
};

union WriteBackStageSignals{
  int16_t raw;
  struct {
    char MemtoReg;
    char RegWrite;
  } data;
};

struct PipelineIFID {
  /* Program counter */
  uint32_t pc;
  /* Fetched instruction */
  int32_t inst;
} pipelineIFID;

struct PipelineIDEX {
  /* alu input data */
  int32_t reg_read_0, reg_read_1;

  /* immidiate with sign extend */
  int32_t imm_sx;

  /* TODO: rs rt??? */

  /* signals */
  /** Execute stage signals */
  union {
    int32_t raw;
    struct {
      char alu_p0_src;
      char alu_p1_src;
      char alu_op;
      char reg_dst;
    } data;
  } SIGEX;
  union MemoryStageSignals SIGMEM;
  union WriteBackStageSignals SIGWB;
} pipelineIDEX;

struct PipelineEXMEM {
  /* result of alu arithmatic operation on program counter */
  uint32_t alu_pc_result;

  int32_t alu_result;

  int32_t reg_read_1;

  int32_t reg_read_from_decode;	/* TODO: what?????????? */

  union MemoryStageSignals SIGMEM;
  union WriteBackStageSignals SIGWB;
};

struct PipelineMEMWB {
  int32_t alu_result;
  int32_t mem_read;

  int32_t reg_read_from_decode;	/* TODO: pls fix ;_; */

  union WriteBackStageSignals SIGWB;
};

#endif
