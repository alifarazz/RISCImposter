#ifndef IMPOSTER_ARCH_MIPS_HAZARD_H
#define IMPOSTER_ARCH_MIPS_HAZARD_H

#include <stdio.h>

#include "./defs.h"
#include "./memory.h"

void hazard_itype(int32_t inst, int32_t pc)
{
  int32_t rt = get_rt(inst), opcode = get_opcode(inst);

  if (opcode == OPCODE_BEQ)
    return;

  int32_t nxt_inst = g_instructions[pc + 1], nxxt_inst = g_instructions[pc + 2];
  char nxt_is_rtype = is_rtype(nxt_inst), nxxt_is_rtype = is_rtype(nxxt_inst);
  char nxt_is_itype  = !is_jtype(nxt_inst) && !nxt_is_rtype,
       nxxt_is_itype = !is_jtype(nxxt_inst) && !nxxt_is_rtype;

  if (nxt_is_rtype) {
    int32_t nxt_rd = get_rd(nxt_inst);
    if (nxt_rd && rt == nxt_rd)
      printf("[HAZARD] load use [%d, %d]\n", pc, pc + 1);
  } else if (nxt_is_itype) {
    int32_t nxt_rt = get_rt(nxt_inst);
    if (nxt_rt && rt == nxt_rt)
      printf("[HAZARD] load use [%d, %d]\n", pc, pc + 1);
  }

  if (opcode == OPCODE_SW)
    return;

  if (nxxt_is_rtype) {
    int32_t nxxt_rd = get_rd(nxxt_inst);
    if (nxxt_rd && rt == nxxt_rd)
      printf("[HAZARD] load use [%d, %d]\n", pc, pc + 2);
  } else if (nxxt_is_itype) {
    int32_t nxxt_rt = get_rt(nxxt_inst);
    if (nxxt_rt && rt == nxxt_rt)
      printf("[HAZARD] load use [%d, %d]\n", pc, pc + 2);
  }
}

void hazard_rtype(int32_t inst, int32_t pc)
{
  int32_t rd       = get_rd(inst);
  int32_t nxt_inst = g_instructions[pc + 1], nxxt_inst = g_instructions[pc + 2];
  char nxt_is_rtype = is_rtype(nxt_inst), nxxt_is_rtype = is_rtype(nxxt_inst);
  char nxt_is_itype  = !is_jtype(nxt_inst) && !nxt_is_rtype,
       nxxt_is_itype = !is_jtype(nxxt_inst) && !nxxt_is_rtype;

  if (nxt_is_rtype || nxt_is_itype) {
    int32_t nxt_rs = get_rs(nxt_inst), nxt_rt = get_rt(nxt_inst);
    if ((nxt_rs && rd == nxt_rs) || (nxt_rt && rd == nxt_rt))
      printf("[HAZARD] load use [%d, %d]\n", pc, pc + 1);
  }

  if (nxxt_is_rtype || nxxt_is_itype) {
    int32_t nxxt_rs = get_rs(nxxt_inst), nxxt_rt = get_rt(nxxt_inst);
    if ((nxxt_rs && rd == nxxt_rs) || (nxxt_rt && rd == nxxt_rt))
      printf("[HAZARD] load use [%d, %d]\n", pc, pc + 2);
  }
}

#endif
