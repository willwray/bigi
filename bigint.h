#ifndef BIGINT_H
#define BIGINT_H 1

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <gmp.h>

#define TOSTR(x) #x
#define STR(x) TOSTR(x)

#define GMP_VER_STR \
  STR(__GNU_MP_VERSION)"." \
  STR(__GNU_MP_VERSION_MINOR)"." \
  STR(__GNU_MP_VERSION_PATCHLEVEL);

// Prealloc size: number of bigints preallocated
// prior to reallocation when growth is required
#define BIGINT_PREALLOC_SIZE 1024

typedef mpz_t bigint;

const char* bigint_info = "GNU multi-precision lib GMP v" GMP_VER_STR;

typedef struct
{
  uint32_t size;
  bigint* data;

} bigint_array;

void bigints_clear(bigint_array* b)
{
  for (int i = 0; i != b->size; ++i)
    mpz_clear(b->data[i]);
  free(b->data);
  b->data = NULL;
  b->size = 0;
}

bigint_array bigints_read(FILE* bigint_file)
{
  bigint_array bigints = {};

  if (bigint_file)
  {
    bigints.data = calloc(BIGINT_PREALLOC_SIZE, sizeof(bigint));

    while (gmp_fscanf(bigint_file, "%Zd", bigints.data[bigints.size]) != EOF)
    {
      bigints.size++;
      if ( (bigints.size & (bigints.size-1)) == 0
         && bigints.size >= BIGINT_PREALLOC_SIZE) {
        void* p = realloc(bigints.data, 2 * bigints.size * sizeof(bigint));
        if (p)
          bigints.data = p;
        else
          bigints_clear(&bigints);
      }
    }
  }
  return bigints;
}



void bigints_write(FILE* fs, bigint_array b)
{
  for (int i = 0; i != b.size; ++i) {
    gmp_fprintf(fs,"%Zx,",b.data[i]);
  }
}

#undef BIGINT_PREALLOC_SIZE
#undef GMP_VER_STR
#undef STR
#undef TOSTR

#endif
