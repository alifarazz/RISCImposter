#ifndef IMPOSTER_ARCH_MIPS_STAGES_H
#define IMPOSTER_ARCH_MIPS_STAGES_H

#include "./compnents.h"
#include "./defs.h"
#include "./memory.h"
#include "./pipeline.h"

/* two structs for each pipeline. pipeline****p will hold old values.
 * pipeline****n will hold new values. at the negedge of clock,
 * p piplines will be updated. */
struct PipelineIFID pipelineIFIDp, pipelineIFIDn;
struct PipelineIDEX pipelineIDEXp, pipelineIDEXn;
struct PipelineEXMEM pipelineEXMEMp, pipelineEXMEMn;
struct PipelineMEMWB pipelineMEMWBp, pipelineMEMWBn;

void exec_stage_fetch()
{
  pipelineIFIDn.inst = g_instructions[prgc]; /* fetch instruction from instruction memroy */

  prgc = alu(prgc, 4 / 4, FUNC_ADD, 0);

  pipelineIFIDn.pc = prgc;
}

/* should be called after fetch stage */
void exec_stage_decode()
{
  static int remaining_pipeline_stalls = 0;

  const int32_t inst   = pipelineIFIDp.inst;
  const int32_t opcode = get_opcode(inst);

  if (remaining_pipeline_stalls) {
    remaining_pipeline_stalls--;

    prgc = alu(prgc, 1, FUNC_SUB, 0); /* pc - 1 */

    pipelineIDEXn.SIGWB.data.RegWrite   = 0;
    pipelineIDEXn.SIGWB.data.MemtoReg   = 0;
    pipelineIDEXn.SIGEX.data.alu_p0_src = 0;
    pipelineIDEXn.SIGEX.data.alu_p1_src = 0;
    pipelineIDEXn.SIGEX.data.reg_dst    = 0;
    pipelineIDEXn.SIGEX.data.alu_op     = 0;
    pipelineIDEXn.SIGMEM.data.MemRead   = 0;
    pipelineIDEXn.SIGMEM.data.MemWrite  = 0;
  }

  if (opcode == 0x00000000) { /* nop */
    pipelineIDEXn.SIGWB.data.RegWrite   = 0;
    pipelineIDEXn.SIGWB.data.MemtoReg   = 0;
    pipelineIDEXn.SIGEX.data.alu_p0_src = 0;
    pipelineIDEXn.SIGEX.data.alu_p1_src = 0;
    pipelineIDEXn.SIGEX.data.reg_dst    = 0;
    pipelineIDEXn.SIGEX.data.alu_op     = 0;
    pipelineIDEXn.SIGMEM.data.MemRead   = 0;
    pipelineIDEXn.SIGMEM.data.MemWrite  = 0;
  }

  /* read reg contents for alu input in exec stage */
  pipelineIDEXn.reg_read_0 = g_regfile[get_rt(inst)];
  pipelineIDEXn.reg_read_1 = g_regfile[get_rs(inst)];

  /* seperate the immidiate from the instruction */
  pipelineIDEXn.imm_sx = sign_extend(get_imm(inst));

  pipelineIDEXn.reg_write_dest = get_rd(inst);
  pipelineIDEXn.reg_write_trgt = get_rt(inst);

  /* Signals are handled per insturction type except for
   * the i-type instructions. They each will be handeled separately */
  /* {add, sub, and, or, slt, nor, xor} are handled here */
  if (is_rtype(inst)) {
    /* set WB stage signals */
    pipelineIDEXn.SIGWB.data.RegWrite = 1; /* write alu result to regfile */
    pipelineIDEXn.SIGWB.data.MemtoReg = 0; /* don't write the read value to regfile */
    /* set EX stage signals
     * these signals are used for execution and forwarding operations */
    pipelineIDEXn.SIGEX.data.alu_p0_src = get_rs(inst);
    pipelineIDEXn.SIGEX.data.alu_p1_src = get_rt(inst);
    pipelineIDEXn.SIGEX.data.reg_dst    = get_rd(inst);
    /* set proper alu operation */
    pipelineIDEXn.SIGEX.data.alu_op = get_func(inst);
    /* set MEM stage signals: bypass memory */
    pipelineIDEXn.SIGMEM.data.MemRead  = 0;
    pipelineIDEXn.SIGMEM.data.MemWrite = 0;
  } else if (is_jtype(inst)) { /* only j instruction is supported */
    /* nop the fetched instruction */
    pipelineIFIDn.inst = 0x00000000;

    /* set new pc extracted from instruction. copy the 6 msbs of the pc and fill
     * the rest with instruction's address. mask 2 msbs of pc beacuse the instruction
     * addresses are word aligned and previously divided by 4 */
    prgc = (get_address_j(inst) ^ (prgc & (~J_ADDR_MASK))) & 0x3FFFFFFF;

    pipelineIDEXn.SIGWB.data.RegWrite   = 0; /* don't alu result to regfile */
    pipelineIDEXn.SIGWB.data.MemtoReg   = 0; /* don't write the read value to regfile */
    pipelineIDEXn.SIGEX.data.alu_p0_src = 0;
    pipelineIDEXn.SIGEX.data.alu_p1_src = 0;
    pipelineIDEXn.SIGEX.data.reg_dst    = 0;
    pipelineIDEXn.SIGEX.data.alu_op     = 0; /* doesn't matter */
    pipelineIDEXn.SIGMEM.data.MemRead   = 0;
    pipelineIDEXn.SIGMEM.data.MemWrite  = 0;
  } else {
    /* here we handle the i-types */
    if (opcode == OPCODE_BEQ) { /* branch equal */
      if (pipelineIDEXn.reg_read_0 == pipelineIDEXn.reg_read_1) {
	prgc = alu(prgc, pipelineIDEXn.imm_sx / 4, FUNC_ADD, 0); /* pc + beq's imm / 4 */
	prgc = alu(prgc, 1, FUNC_ADD, 0);			 /* pc + 1 */

	pipelineIFIDn.inst = 0x00000000; /* nop */

	pipelineIDEXn.SIGWB.data.RegWrite   = 0; /* don't alu result to regfile */
	pipelineIDEXn.SIGWB.data.MemtoReg   = 0; /* don't write the read value to regfile */
	pipelineIDEXn.SIGEX.data.alu_p0_src = 0;
	pipelineIDEXn.SIGEX.data.alu_p1_src = 0;
	pipelineIDEXn.SIGEX.data.reg_dst    = 0;
	pipelineIDEXn.SIGEX.data.alu_op     = 0; /* doesn't matter */
	pipelineIDEXn.SIGMEM.data.MemRead   = 0;
	pipelineIDEXn.SIGMEM.data.MemWrite  = 0;
      }
    } else if (opcode == OPCODE_SW) { /* store word */
      /* we stall the pipeline 3times */
      prgc = alu(prgc, 1, FUNC_SUB, 0); /* pc - 1 */

      pipelineIFIDn.inst	= 0x00000000; /* nop */
      remaining_pipeline_stalls = 2;

      pipelineIDEXn.SIGEX.data.alu_p0_src = get_rs(inst);
      pipelineIDEXn.SIGEX.data.alu_p1_src = 0; /* because i-type */
      pipelineIDEXn.SIGEX.data.reg_dst    = get_rt(inst);
      pipelineIDEXn.SIGEX.data.alu_op     = FUNC_ADD;
      pipelineIDEXn.SIGMEM.data.MemRead   = 0;
      pipelineIDEXn.SIGMEM.data.MemWrite  = 1;
      pipelineIDEXn.SIGWB.data.RegWrite   = 1;
      pipelineIDEXn.SIGWB.data.MemtoReg   = 1; /* update regfile */
    } else if (opcode == OPCODE_LW) {	  /* load word */
      /* we stall the pipeline 3times */
      prgc = alu(prgc, 1, FUNC_SUB, 0); /* pc - 1 */

      pipelineIFIDn.inst	= 0x00000000; /* nop */
      remaining_pipeline_stalls = 2;

      pipelineIDEXn.SIGEX.data.alu_p0_src = get_rs(inst);
      pipelineIDEXn.SIGEX.data.alu_p1_src = 0; /* because i-type */
      pipelineIDEXn.SIGEX.data.reg_dst    = get_rt(inst);
      pipelineIDEXn.SIGEX.data.alu_op     = FUNC_ADD;
      pipelineIDEXn.SIGMEM.data.MemRead   = 1;
      pipelineIDEXn.SIGMEM.data.MemWrite  = 0;
      pipelineIDEXn.SIGWB.data.RegWrite   = 1;
      pipelineIDEXn.SIGWB.data.MemtoReg   = 1; /* update regfile */
    } else {				       /* not implemented */
      printf("[WARN] instruction not implemented, stage:%s, PC:%d\n", "DECODE", prgc);
    }
  }
}

void exec_stage_exec()
{
  if (pipelineIDEXp.SIGEX.data.alu_p1_src) { /* is it an i-type? */
    pipelineEXMEMn.alu_result =
	alu(pipelineIDEXp.reg_read_0, pipelineIDEXp.imm_sx, pipelineIDEXp.SIGEX.data.alu_op, 0);
    pipelineEXMEMn.reg_write = pipelineIDEXp.reg_write_trgt;
  } else { /* it's an r-type */
    pipelineEXMEMn.alu_result =
	alu(pipelineIDEXp.reg_read_0, pipelineIDEXp.reg_read_1, pipelineIDEXp.SIGEX.data.alu_op, 0);
    pipelineEXMEMn.reg_write = pipelineIDEXp.reg_write_dest;
  }

  pipelineEXMEMn.reg_read_1 = pipelineIDEXp.reg_read_1;
  pipelineEXMEMn.SIGMEM     = pipelineIDEXp.SIGMEM;
  pipelineMEMWBn.SIGWB      = pipelineIDEXp.SIGWB;
}

void exec_stage_mem()
{
  int err;
  if (pipelineEXMEMp.SIGMEM.data.MemRead && pipelineEXMEMp.SIGMEM.data.MemWrite) {
    printf("[ERROR] encounterd memory read and write. stage: %s, pc: %d", "MEMORY", prgc);
    return;
  } else if (!pipelineEXMEMp.SIGMEM.data.MemRead && pipelineEXMEMp.SIGMEM.data.MemWrite) {
    if ((err = write_memory(pipelineEXMEMp.alu_result, pipelineEXMEMp.reg_read_1)))
      printf("[ERROR] memory %s error:%d\nstage: %s, pc: %d", "write", err, "MEMORY", prgc);
  } else if (pipelineEXMEMp.SIGMEM.data.MemRead && !pipelineEXMEMp.SIGMEM.data.MemWrite) {
    if ((err = read_memory(pipelineEXMEMp.alu_result, &(pipelineEXMEMp.reg_read_1))))
      printf("[ERROR] memory %s error:%d\nstage: %s, pc: %d", "read", err, "MEMORY", prgc);
  } else {
    /* don't touch the memory so nothing */
  }

  pipelineMEMWBn.reg_write = pipelineEXMEMp.reg_write;
  pipelineMEMWBn.SIGWB     = pipelineEXMEMp.SIGWB;
}



/* select reg for write operation in write-back stage */

#endif
