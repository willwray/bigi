#ifndef QUICKSORT_H
#define QUICKSORT_H 1

#include <gmp.h>

//#include <stddef.h>
//#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define MPZ_SHALLOW_ASSIGN(a,b) *a=*b

mpz_t* partition_mpz_t(mpz_t* b, mpz_t* e, mpz_t v, bool neg);
void quicksort_mpz_t(mpz_t* b, mpz_t* e);

/** @brief Partition for given type with baked-in compare.
 *  @param b Begin pointer of input sequence.
 *  @param e End pointer of input sequence.
 *  @param v Value of pivot element.
 *  @param neg Negate the comparison if true.
 */
#define PARTITION(type,compare,swap) \
type* partition_##type(type* b, type* e, type v, bool neg) { \
  for (; b != e; ++b) \
    if (compare(*b,v) == neg) break; \
  printf("%d %d %d,",*b,v,neg); \
  if (b == e) return b; \
  for (type* i = ++b; i != e; ++i) { \
    if (compare(*i,v) != neg) { \
      swap(*i,*b); \
      ++b; \
    } \
  } \
  return b; \
}

#define COMPARE(a,b) ((a)<(b))
#define ASSIGN(a,b) a=(b)
#define ITER_SWAP(a,b) __typeof__(*(a)) tmp = *(a); *(a) = *(b); *(b) = tmp;

int* partition_int(int* b, int* e, int v, bool neg) {
  for (; b != e; ++b)
    if ((*b < v) == neg) break;
  //printf("%d %d %d,",*b,v,neg);
  if (b == e) return b;
  for (int* i = b+1; i != e; ++i) {
    if ((*i < v) != neg) {
      ITER_SWAP(i,b);
      ++b;
    }
  }
  return b;
}

//PARTITION(int,COMPARE,SWAP)
//PARTITION(mpz_t,mpz_cmp,mpz_swap)

/** @brief Unbalanced quicksort for given type.
 *  @param b Begin pointer of input sequence.
 *  @param e End pointer of input sequence.
 */
#define QUICKSORT(type,assign) \
void quicksort_##type(type* b, type* e) { \
  ptrdiff_t n = e - b; \
  if (n < 2) \
    return; \
  type pivot; assign(pivot,*(b + n / 2)); \
  type* mid1 = partition_##type(b,e,pivot,false); \
  type* mid2 = partition_##type(mid1,e,pivot,true); \
  quicksort_##type(b, mid1); \
  quicksort_##type(mid2, e); \
}

QUICKSORT(int,ASSIGN)
//QUICKSORT(mpz_t,MPZ_SHALLOW_ASSIGN)

//printf("%td",n);
#endif
