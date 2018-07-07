#ifndef LOL_UTILS_H
#define LOL_UTILS_H

#include <stdio.h>
#include <stdlib.h>

union ConverterIntChar {
  int32_t i;
  int8_t c[4];
};

/* my own "safe" malloc*/
void*
lol_malloc(size_t length, size_t size)
{
  size_t x = length * size;
  if (length != 0 && x / length != size) { /* check for overflow */
    fprintf(stderr,
	    "malloc failed.\n%s\n\tlength: %lu, size: %lu\n",
	    "Reason: size_t overflow", length, size);
    exit(EXIT_FAILURE);
  }
  void* mp = malloc(x);
  if (mp == NULL) {
    fprintf(stderr,
	    "malloc failed.\n%s\n\t length: %lu, size: %lu\n",
	    "malloc returned NULL", length, size);
    exit(EXIT_FAILURE);
  }
  return mp;
}


static FILE*
open_file(const char* location, const char *mode){
  FILE *fp;
  if ((fp = fopen(location, mode)) == NULL) {
    perror("fopen() failed");
  }
  return fp;
}
#endif
