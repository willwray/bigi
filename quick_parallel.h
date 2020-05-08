#ifndef PARALLEL_QUICKSORT_H
#define PARALLEL_QUICKSORT_H 1

struct __mpz_struct;
typedef __mpz_struct mpz_t[1];
using type = mpz_t;
using iter = type*;

#include <stddef.h>
#include <stdlib.h>

#include "partition.h"

#define sort_qs_num_samples_preset 100

// https://stackoverflow.com/questions/3437404/min-and-max-in-c
#define min(a,b) \
   ({ __typeof__(a) _a = (a); \
      __typeof__(b) _b = (b); \
     _a < _b ? _a : _b; })

/** @brief Unbalanced quicksort divide step.
 *  @param b Begin iterator of subsequence.
 *  @param e End iterator of subsequence.
 *  @param compare Comparator.
 *  @param pivot_rank Desired __rank of the pivot.
 *  @param num_samples Choose pivot from that many samples.
 *  @param num_threads Number of threads allowed to work on this part.
 */
ptrdiff_t parallel_sort_qs_divide(iter b, iter e,
          ptrdiff_t pivot_rank, ptrdiff_t num_samples, int num_threads)
{
  ptrdiff_t n = e - b;
  num_samples = min(num_samples, n);

  type* samples = (type*)malloc(num_samples * sizeof(type));

  for (ptrdiff_t s = 0; s < num_samples; ++s) {
    const unsigned long long ind = (unsigned long long)(s) * n / num_samples;
    samples[s] = b[ind];
  }

  sequential_sort(samples, samples + num_samples);
  type* pivot = &samples[pivot_rank * num_samples / n];

  ptrdiff_t split = parallel_partition(b, e, num_threads);
  for (ptrdiff_t s = 0; s < num_samples; ++s)
     samples[s].~type();
     
  delete(samples);
  return split;
}

/** @brief Unbalanced quicksort conquer step.
 *  @param b Begin iterator of subsequence.
 *  @param e End iterator of subsequence.
 *  @param compare Comparator.
 *  @param num_threads Number of threads allowed to work on this part.
 */
void parallel_sort_qs_conquer(iter b, iter e, int num_threads)
{
  if (num_threads <= 1) {
    sequential_sort(b, e);
    return;
  }
  ptrdiff_t n = e - b, pivot_rank;

  if (n <= 1)
    return;
  int num_threads_left;
  if ((num_threads % 2) == 1)
    num_threads_left = num_threads / 2 + 1;
  else
    num_threads_left = num_threads / 2;

  pivot_rank = n * num_threads_left / num_threads;
  ptrdiff_t split = parallel_sort_qs_divide(b, e,pivot_rank,
                      sort_qs_num_samples_preset, num_threads);
#pragma omp parallel sections num_threads(2)
  {
#pragma omp section
    parallel_sort_qs_conquer(b, b + split, num_threads_left);
#pragma omp section
    parallel_sort_qs_conquer(b + split, e, num_threads - num_threads_left);
  }
}

/** @brief Unbalanced quicksort main call.
 *  @param b Begin iterator of input sequence.
 *  @param e End iterator input sequence, ignored.
 *  @param compare Comparator.
 *  @param num_threads Number of threads allowed to work on this part.
 */
void parallel_sort_qs(iter b, iter e, int num_threads)
{
  // At least one element per processor.
  ptrdiff_t n = e - b;
  if (num_threads > n)
      num_threads = (int)n;
  parallel_sort_qs_conquer(b, b + n, num_threads);
}

#endif /* PARALLEL_QUICKSORT_H */