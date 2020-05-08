#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define LESS_THAN(a,b) ((a) < (b))

#define MERGE(type,compare) \
type* merge_##type(type* b1, type* e1, type* b2, type* e2, type* o) { \
  for (; b1 != e1; ++o) { \
    if (b2 == e2) { \
      while (b1 != e1) \
        *o++ = *b1++; \
      return o; \
    } \
    type** c = compare(*b2, *b1) ? &b2 : &b1; \
    *o = **c; ++*c; \
  } \
  while (b2 != e2) \
    *o++ = *b2++; \
  return o; \
}

#define compare(a,b) LESS_THAN(a,b)
typedef int type;
