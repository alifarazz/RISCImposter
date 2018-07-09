#ifndef IMPOSTER_ARCH_MIPS_STAGES_H
#define IMPOSTER_ARCH_MIPS_STAGES_H

#include "./compnents.h"
#include "./defs.h"
#include "./hazard.h"
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
  printf("FETCH prgc: 0x%x\n", prgc);
  prgc = alu(prgc, 4 / 4, FUNC_ADD, 0);

  pipelineIFIDn.pc = prgc;

  g_regfile[0] = 0x00000000; /* $zero reg should always be 0 */
}

/* should be called after fetch stage */
void exec_stage_decode()
{
  static int remaining_pipeline_stalls = 0;

  const int32_t inst   = pipelineIFIDp.inst;
  const int32_t opcode = get_opcode(inst);


  /* hacky way of stalling the pipeline more than once */
  if (remaining_pipeline_stalls > 0) {
    /* stall the pipeline */
    if (remaining_pipeline_stalls != 1) {
      prgc		 = alu(prgc, 1, FUNC_SUB, 0); /* pc - 1 */
      pipelineIFIDn.inst = 0x00000000;
    }

    pipelineIDEXn.SIGWB.data.RegWrite   = 0;
    pipelineIDEXn.SIGWB.data.MemtoReg   = 0;
    pipelineIDEXn.SIGEX.data.alu_p0_src = 0;
    pipelineIDEXn.SIGEX.data.alu_p1_src = 0;
    pipelineIDEXn.SIGEX.data.reg_dst    = 0;
    pipelineIDEXn.SIGEX.data.alu_op     = 0;
    pipelineIDEXn.SIGMEM.data.MemRead   = 0;
    pipelineIDEXn.SIGMEM.data.MemWrite  = 0;
    remaining_pipeline_stalls--;
    return;
  }

  if (inst == 0x00000000) { /* process nop separately */
    pipelineIDEXn.SIGWB.data.RegWrite   = 0;
    pipelineIDEXn.SIGWB.data.MemtoReg   = 0;
    pipelineIDEXn.SIGEX.data.alu_p0_src = 0;
    pipelineIDEXn.SIGEX.data.alu_p1_src = 0;
    pipelineIDEXn.SIGEX.data.reg_dst    = 0;
    pipelineIDEXn.SIGEX.data.alu_op     = 0;
    pipelineIDEXn.SIGMEM.data.MemRead   = 0;
    pipelineIDEXn.SIGMEM.data.MemWrite  = 0;
    return;
  }

  /* read reg contents for alu input in exec stage */
  pipelineIDEXn.reg_read_0 = g_regfile[get_rs(inst)];
  pipelineIDEXn.reg_read_1 = g_regfile[get_rt(inst)];

  /* seperate the immidiate from the instruction */
  pipelineIDEXn.imm_sx = sign_extend(get_imm(inst));

  pipelineIDEXn.reg_write_dest = get_rd(inst);
  pipelineIDEXn.reg_write_trgt = get_rt(inst);

  /* Signals are handled per insturction type except for
   * the i-type instructions. They each will be handeled separately */
  /* {add, sub, and, or, slt, nor, xor} are handled here */
  if (is_rtype(inst)) {
    /* check hazard */
    prgc = alu(prgc, 1, FUNC_SUB, 0); /* pc - 1 */

    pipelineIFIDn.inst = 0x00000000; /* nop the fetched instruction */
    printf("DECODE opcode: %x, is rtype: %d\n", opcode, 1);
    remaining_pipeline_stalls = 2;

    /* set WB stage signals */
    pipelineIDEXn.SIGWB.data.RegWrite = 1; /* write alu result to regfile */
    pipelineIDEXn.SIGWB.data.MemtoReg = 0; /* don't write the read value from memory to regfile */
    /* set EX stage signals
     * these signals are used for execution and forwarding operations */
    pipelineIDEXn.SIGEX.data.alu_p0_src = get_rs(inst);
    pipelineIDEXn.SIGEX.data.alu_p1_src = get_rt(inst);
    pipelineIDEXn.SIGEX.data.reg_dst    = get_rd(inst);
    printf("DECODE rs: 0x%x, rt: 0x%x, rd 0x%x\n", get_rs(inst), get_rt(inst), get_rd(inst));
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
     * addresses are word aligned and previously divided by 4. add prgc by 5 to
     * compensate for initial nops. */
    prgc = ((get_address_j(inst) ^ (prgc & (~J_ADDR_MASK))) & 0x03FFFFFF) + 5;

    printf("1DECODE: jtype: prgc: 0x%x, cpd_addr: 0x%x\n", prgc,
	   ((get_address_j(inst) ^ (prgc & (~J_ADDR_MASK))) & 0x03FFFFFF) + 5);

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
	prgc = alu(
	    prgc,
	    (0 < pipelineIDEXn.imm_sx) ? -(-pipelineIDEXn.imm_sx) : pipelineIDEXn.imm_sx,
	    FUNC_ADD, 0);		  /* pc + beq's imm / 4 */
	prgc = alu(prgc, 1, FUNC_ADD, 0); /* pc + 1 */
	printf("DECODE beq, new prgc: 0x%x\n", prgc);
	pipelineIFIDn.inst = 0x00000000; /* nop the previous stage */
      }
      pipelineIDEXn.SIGWB.data.RegWrite   = 0; /* don't alu result to regfile */
      pipelineIDEXn.SIGWB.data.MemtoReg   = 0; /* don't write the read value to regfile */
      pipelineIDEXn.SIGEX.data.alu_p0_src = 0;
      pipelineIDEXn.SIGEX.data.alu_p1_src = 0;
      pipelineIDEXn.SIGEX.data.reg_dst    = 0;
      pipelineIDEXn.SIGEX.data.alu_op     = 0; /* doesn't matter */
      pipelineIDEXn.SIGMEM.data.MemRead   = 0;
      pipelineIDEXn.SIGMEM.data.MemWrite  = 0;
    } else if (opcode == OPCODE_SW) { /* store word */
      /* we stall the pipeline 3times */
      prgc = alu(prgc, 1, FUNC_SUB, 0); /* pc - 1 */

      pipelineIFIDn.inst = 0x00000000; /* nop IF stage */

      remaining_pipeline_stalls = 1; /* nop until sw finishes memory read */

      printf("DECODE store word addr: 0x%x\n", pipelineIDEXn.imm_sx + pipelineIDEXn.reg_read_0);

      pipelineIDEXn.SIGEX.data.alu_p0_src = get_rs(inst);
      pipelineIDEXn.SIGEX.data.alu_p1_src = 0; /* because i-type */
      pipelineIDEXn.SIGEX.data.reg_dst    = get_rt(inst);
      pipelineIDEXn.SIGEX.data.alu_op     = FUNC_ADD;
      pipelineIDEXn.SIGMEM.data.MemRead   = 0;
      pipelineIDEXn.SIGMEM.data.MemWrite  = 1;
      pipelineIDEXn.SIGWB.data.RegWrite   = 1;
      pipelineIDEXn.SIGWB.data.MemtoReg   = 0; /* don't touch regfile */
    } else if (opcode == OPCODE_LW) {	  /* load word */
      /* we stall the pipeline 3times */
      prgc = alu(prgc, 1, FUNC_SUB, 0); /* pc - 1 */

      pipelineIFIDn.inst = 0x00000000; /* nop IF stage*/
      /* nop until lw finishes write-back
       * write-back stage happens before decode stage */
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
  discover_hazards();
}

void exec_stage_exec()
{
  if (pipelineIDEXp.SIGEX.data.alu_p1_src || pipelineIDEXp.SIGEX.data.alu_p0_src)
    printf("EXEC: src0: %d: 0x%x, src1: %d: 0x%x\n", pipelineIDEXp.SIGEX.data.alu_p0_src,
	   pipelineIDEXp.reg_read_0, pipelineIDEXp.SIGEX.data.alu_p1_src, pipelineIDEXp.reg_read_1);

  if (!pipelineIDEXp.SIGEX.data.alu_p1_src) { /* is it an i-type? */
    pipelineEXMEMn.alu_result =
	alu(pipelineIDEXp.reg_read_0, pipelineIDEXp.imm_sx, pipelineIDEXp.SIGEX.data.alu_op, 0);
    pipelineEXMEMn.reg_write = pipelineIDEXp.reg_write_trgt;
  } else { /* it's an r-type */
    pipelineEXMEMn.alu_result =
	alu(pipelineIDEXp.reg_read_0, pipelineIDEXp.reg_read_1, pipelineIDEXp.SIGEX.data.alu_op, 0);
    pipelineEXMEMn.reg_write = pipelineIDEXp.reg_write_dest;
    printf("EXEC: result:0x%x\t rd: %d\n", pipelineEXMEMn.alu_result, pipelineEXMEMn.reg_write);
  }

  pipelineEXMEMn.reg_read_1 = pipelineIDEXp.reg_read_1;
  pipelineEXMEMn.SIGMEM.raw = pipelineIDEXp.SIGMEM.raw;
  pipelineEXMEMn.SIGWB.raw  = pipelineIDEXp.SIGWB.raw;
}

void exec_stage_mem()
{
  int err;
  printf("MEMread:%d, MEMwrite: %d\n", pipelineEXMEMp.SIGMEM.data.MemRead,
	 pipelineEXMEMp.SIGMEM.data.MemWrite);
  if (pipelineEXMEMp.SIGMEM.data.MemRead && pipelineEXMEMp.SIGMEM.data.MemWrite) {
    printf("[ERROR] encounterd memory read and write.\nstage: %s, pc: %d", "MEMORY", prgc);
    return;
  } else if (!pipelineEXMEMp.SIGMEM.data.MemRead && pipelineEXMEMp.SIGMEM.data.MemWrite) {
    if ((err = write_memory(pipelineEXMEMp.alu_result, pipelineEXMEMp.reg_read_1)))
      printf("[ERROR] %s error:%d\nstage: %s, pc: %d", "write", err, "MEMORY", prgc);
  } else if (pipelineEXMEMp.SIGMEM.data.MemRead && !pipelineEXMEMp.SIGMEM.data.MemWrite) {
    if ((err = read_memory(pipelineEXMEMp.alu_result, &(pipelineMEMWBn.mem_read))))
      printf("[ERROR] %s error:%d\nstage: %s, pc: %d", "read", err, "MEMORY", prgc);
  } else {
    /* don't touch the memory so nothing */
  }

  pipelineMEMWBn.alu_result = pipelineEXMEMp.alu_result;
  pipelineMEMWBn.reg_write  = pipelineEXMEMp.reg_write;
  printf("MEM: alu_result: %d, reg_write: 0x%x\n", pipelineMEMWBn.alu_result,
	 pipelineMEMWBn.reg_write);
  pipelineMEMWBn.SIGWB.raw = pipelineEXMEMp.SIGWB.raw;
}

void exec_stage_wb()
{
  if (pipelineMEMWBp.SIGWB.data.RegWrite) {
    if (pipelineMEMWBp.SIGWB.data.MemtoReg) { /* i type? */
      puts("wb itype");
      g_regfile[pipelineMEMWBp.reg_write] = pipelineMEMWBp.mem_read;
    } else { /* r type */
      puts("wb rtype");
      g_regfile[pipelineMEMWBp.reg_write] = pipelineMEMWBp.alu_result;
    }
  }
}

#endif
