#ifndef LOL_MEMORY_H
#define LOL_MEMORY_H

int
lol_read_memory(const struct Context ctx,
		const int32_t idx, int32_t *content)
{
  union ConverterIntChar convert_ic;
  if (!(ctx.mainmem)) return -1;	      /* invalid context */
  if (!content) return -2;			      /* invalid content */
  if (idx+3 >= ctx.mainmem_size) return 1;	      /* out of memeory */

  /* Read 4 bytes from main memory and convert it to a 4 byte word */
#ifdef __BIG_ENDIAN__
  convert_ic.c[0] = (ctx.mainmem)[idx];
  convert_ic.c[1] = (ctx.mainmem)[idx + 1];
  convert_ic.c[2] = (ctx.mainmem)[idx + 2];
  convert_ic.c[3] = (ctx.mainmem)[idx + 3];
#else
  convert_ic.c[0] = (ctx.mainmem)[idx + 3];
  convert_ic.c[1] = (ctx.mainmem)[idx + 2];
  convert_ic.c[2] = (ctx.mainmem)[idx + 1];
  convert_ic.c[3] = (ctx.mainmem)[idx];
#endif
  *content = convert_ic.i;
  return 0;
}

int
lol_write_memory(struct Context *ctx, int32_t idx, int32_t content)
{
  union ConverterIntChar convert_ic;
  if (!ctx || !(ctx->mainmem)) return -1;  /* invalid context */
  if (idx+3 >= ctx->mainmem_size) return 1; /* out of memory */

  /* Write 'content' to 4 consecutive byte at main memory */
  convert_ic.i = content;
#ifdef __BIG_ENDIAN__
  (ctx->mainmem)[idx]     = convert_ic.c[0];
  (ctx->mainmem)[idx + 1] = convert_ic.c[1];
  (ctx->mainmem)[idx + 2] = convert_ic.c[2];
  (ctx->mainmem)[idx + 3] = convert_ic.c[3];
#else
  (ctx->mainmem)[idx + 3] = convert_ic.c[0];
  (ctx->mainmem)[idx + 2] = convert_ic.c[1];
  (ctx->mainmem)[idx + 1] = convert_ic.c[2];
  (ctx->mainmem)[idx]     = convert_ic.c[3];
#endif
  return 0;
}

#endif
