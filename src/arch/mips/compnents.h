#ifndef IMPOSTER_ARCH_MIPS_COMPONENTS_H
#define IMPOSTER_ARCH_MIPS_COMPONENTS_H

/* This file contains most combinational components of
 * mips arch. Combinational circuits are implemented as
 * functions. */

#include <stdint.h>
#include <stdio.h>

#include "./defs.h"

int32_t sign_extend(int16_t imm) { return imm; }

int32_t shift_left(int32_t addr) { return (0x3FFFFFFF & addr) << 2; }

int32_t alu(int32_t a, int32_t b, int func, int shamt)
{
  switch (func) {
  case (FUNC_ADD): {
    /* @TODO : handle int overflow */
    /* printf("alu add called %d\n", a + b); */
    return a + b;
    break;
  }
  case (FUNC_AND): {
    return a & b;
    break;
  }
  case (FUNC_NOR): {
    return ~(a | b);
    break;
  }
  case (FUNC_OR): {
    return a | b;
    break;
  }
  case (FUNC_SLT): {
    return a < b;
    break;
  }
  case (FUNC_SUB): {
    /* @TODO : handle int overflow */
    return a - b;
    break;
  }
  case (FUNC_XOR): {
    return a ^ b;
    break;
  }
  case (0): { /* nop */
    return 0;
    break;
  }
  default:
    /* not implemented yet */
    printf("ALU function not implemented yet, a = %d, b = %d, func = %d, shamt = %d\n", a, b, func,
	   shamt);
    break;
  }
  /* default fall-back value */
  return 0;
}

#endif
