#ifndef HEAPSORT_H
#define HEAPSORT_H 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PUSH_HEAP(type,compare) \
void push_heap(type* b,	ptrdiff_t holeInd, ptrdiff_t topInd, type val) { \
  ptrdiff_t parent = (holeInd - 1) / 2; \
  while (holeInd > topInd && compare(*(b + parent), val)) { \
    *(b + holeInd) = *(b + parent); \
    holeInd = parent; \
    parent = (holeInd - 1) / 2; \
  } \
  *(b + holeInd) = val; \
}

#define ADJUST_HEAP(type,compare) \
void adjust_heap(type* b, ptrdiff_t holeInd, ptrdiff_t len, type val) { \
  const ptrdiff_t topInd = holeInd; \
  ptrdiff_t secondChild = holeInd; \
  while (secondChild < (len - 1) / 2) { \
    secondChild = 2 * (secondChild + 1); \
    if (compare(b + secondChild, b + (secondChild - 1))) \
      secondChild--; \
    *(b + holeInd) = *(b + secondChild); \
    holeInd = secondChild; \
  } \
  if ((len & 1) == 0 && secondChild == (len - 2) / 2) { \
    secondChild = 2 * (secondChild + 1); \
    *(b + holeInd) = *(b + (secondChild - 1)); \
    holeInd = secondChild - 1; \
  } \
  push_heap##type(b, holeInd, topInd, val); \
}

#define MAKE_HEAP(type,compare) \
void make_heap_##type(type* b, type* e) { \
  if (e - b < 2) return; \
  const ptrdiff_t len = e - b; \
  ptrdiff_t parent = (len - 2) / 2; \
  while (true) { \
    adjust_heap(b, parent, len, *(b + parent)); \
    if (parent == 0) \
      return; \
    parent--; \
  } \
}

#endif