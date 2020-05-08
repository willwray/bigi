#ifndef PARTITION_H
#define PARTITION_H 1

#include <omp.h>

#include <stddef.h>
#include <stdlib.h>

#define iter_swap(a,b) __typeof__(*(a)) tmp = *(a); *(a) = *b; *(b) = tmp;

#define swap_ranges(b1, e1, b2) {\
 __typeof__(b1) i1 = (b1); \
 __typeof__(b2) i2 = (b2); \
 while (i1 != e1) { \
  iter_swap(i1++, i2++); \
 } \
}

#define partition_chunk_size 1024
#define partition_chunk_share 0.0

/** @brief Add a value to a variable, atomically.
 *
 *  @param ptr Pointer to a signed integer.
 *  @param addend Value to add.
 */
inline ptrdiff_t fetch_and_add(volatile ptrdiff_t* ptr, ptrdiff_t addend)
{
  if (__atomic_always_lock_free(sizeof(ptrdiff_t), ptr))
    return __atomic_fetch_add(ptr, addend, __ATOMIC_ACQ_REL);
  int64_t res;
# pragma omp critical
  {
    res = *ptr;
    *(ptr) += addend;
  }
  return res;
}

/** @brief Compare-and-swap
 *
 * Compare @c *ptr and @c comparand. If equal, let @c
 * *ptr=replacement and return @c true, return @c false otherwise.
 *
 *  @param ptr Pointer to signed integer.
 *  @param comparand Compare value.
 *  @param replacement Replacement value.
 */
inline bool compare_and_swap(volatile int* ptr, int comparand, int replacement)
{
  if (__atomic_always_lock_free(sizeof(int), ptr))
    return __atomic_compare_exchange_n(ptr, &comparand, replacement,
                    false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED);
  bool res = false;
#pragma omp critical
  {
    if (*ptr == comparand)
    {
      *ptr = replacement;
      res = true;
    }
  }
  return res;
}


using type = int;
using iter = type*;

// b, e , pred?

/** @brief Parallel implementation of partition.
  *  @param b Begin iterator of input sequence to split.
  *  @param e End iterator of input sequence to split.
  *  @param pred Partition predicate, possibly including pivot.
  *  @param num_threads Maximum number of threads to use for this task.
  *  @return Number of elements not fulfilling the predicate.
  */
ptrdiff_t parallel_partition(iter b, iter e, int num_threads)
{
  ptrdiff_t n = e - b;

  // shared
  volatile ptrdiff_t left = 0, right = n - 1, dist = n,
                     leftover_left, leftover_right, leftnew, rightnew;

  // just 0 or 1, but int to allow atomic operations
  int *reserved = 0, *reserved_left = 0, *reserved_right = 0;

  ptrdiff_t chunk_size = partition_chunk_size;

  //at least two chunks per thread
  if (dist >= 2 * num_threads * chunk_size)
# pragma omp parallel num_threads(num_threads)
  {
# pragma omp single
  {
    num_threads = omp_get_num_threads();

    int* reserved = (int*)malloc(2 * num_threads * sizeof(int));
    reserved_left = reserved;
    reserved_right = &reserved[num_threads];

    if (partition_chunk_share > 0.0)
                   // max
      chunk_size = ({ ptrdiff_t a = partition_chunk_size;
                      ptrdiff_t b = (double)n * partition_chunk_share / (double)num_threads;
                      a > b ? a : b; });
    else
      chunk_size = partition_chunk_size;
  }

  while (dist >= 2 * num_threads * chunk_size)
  {
#  pragma omp single
   {
    ptrdiff_t num_chunks = dist / chunk_size;

    for (int r = 0; r < num_threads; ++r)
    {
      reserved_left [r] = 0; // false
      reserved_right[r] = 0; // false
    }
    leftover_left = 0;
    leftover_right = 0;
   } //implicit barrier

  // Private.
  ptrdiff_t thread_left, thread_left_border,
            thread_right, thread_right_border;

  thread_left = left + 1;
  // Just to satisfy the condition below.
  thread_left_border = thread_left - 1;

  thread_right = n - 1;
  // Just to satisfy the condition below.
  thread_right_border = thread_right + 1;

  bool iam_finished = false;
  while (!iam_finished)
  {
    if (thread_left > thread_left_border)
    {
      ptrdiff_t __former_dist =
              fetch_and_add(&dist, -chunk_size);
      if (__former_dist < chunk_size)
      {
        fetch_and_add(&dist, chunk_size);
        iam_finished = true;
        break;
      }
      else
      {
        thread_left =
                fetch_and_add(&left, chunk_size);
        thread_left_border =
                thread_left + (chunk_size - 1);
      }
    }

    if (thread_right < thread_right_border)
    {
      ptrdiff_t __former_dist =
              fetch_and_add(&dist, -chunk_size);
      if (__former_dist < chunk_size)
      {
        fetch_and_add(&dist, chunk_size);
        iam_finished = true;
        break;
      }
      else
      {
        thread_right =
                fetch_and_add(&right, -chunk_size);
        thread_right_border =
                thread_right - (chunk_size - 1);
      }
    }

    // Swap as usual.
    while (thread_left < thread_right)
    {
      while (__pred(b[thread_left])
          && thread_left <= thread_left_border)
          ++thread_left;
      while (!__pred(b[thread_right])
          && thread_right >= thread_right_border)
          --thread_right;

      if (thread_left > thread_left_border
       || thread_right < thread_right_border)
      // Fetch new chunk(__s).
        break;

      iter_swap(b + thread_left, b + thread_right);

      ++thread_left;
      --thread_right;
    }
  }

  // Now swap the leftover chunks to the right places.
  if (thread_left <= thread_left_border)
#   pragma omp atomic
    ++leftover_left;
    if (thread_right >= thread_right_border)
#   pragma omp atomic
    ++leftover_right;

#   pragma omp barrier

    ptrdiff_t
          leftold = left,
          leftnew = left - leftover_left * chunk_size,
          rightold = right,
          rightnew = right + leftover_right * chunk_size;

    // <=> thread_left_border + (chunk_size - 1) >= leftnew
    if (thread_left <= thread_left_border
     && thread_left_border >= leftnew)
    {
      // Chunk already in place, reserve spot.
      reserved_left[(left - (thread_left_border + 1))
                    / chunk_size] = 1;
    }

    // <=> thread_right_border - (chunk_size - 1) <= rightnew
    if (thread_right >= thread_right_border
     && thread_right_border <= rightnew)
    {
      // Chunk already in place, reserve spot.
      reserved_right[((thread_right_border - 1) - right)
                     / chunk_size] = 1;
    }

#   pragma omp barrier

    if (thread_left <= thread_left_border
     && thread_left_border < leftnew)
    {
      // Find spot and swap.
      ptrdiff_t __swapstart = -1;
      for (int r = 0; r < leftover_left; ++r)
        if (reserved_left[r] == 0
            && compare_and_swap(&(reserved_left[r]), 0, 1))
        {
          __swapstart = leftold - (r + 1) * chunk_size;
          break;
        }

      swap_ranges(b + thread_left_border
           - (chunk_size - 1),
           b + thread_left_border + 1,
           b + __swapstart);
    }

    if (thread_right >= thread_right_border
     && thread_right_border > rightnew)
    {
      // Find spot and swap
      ptrdiff_t __swapstart = -1;
      for (int r = 0; r < leftover_right; ++r)
        if (reserved_right[r] == 0
            && compare_and_swap(&(reserved_right[r]), 0, 1))
        {
          __swapstart = rightold + r * chunk_size + 1;
          break;
        }

      swap_ranges(b + thread_right_border,
         b + thread_right_border
         + chunk_size, b + __swapstart);
      }

      left = leftnew;
      right = rightnew;
      dist = right - left + 1;
    }

#   pragma omp flush(left, right)
	} // end "recursion" //parallel

  ptrdiff_t final_left = left, final_right = right;

  while (final_left < final_right)
  {
    // Go right until key is geq than pivot.
    while (__pred(b[final_left])
     && final_left < final_right)
      ++final_left;

    // Go left until key is less than pivot.
    while (!__pred(b[final_right])
     && final_left < final_right)
      --final_right;

    if (final_left == final_right)
      break;
    iter_swap(b + final_left, b + final_right);
    ++final_left;
    --final_right;
  }

  // All elements on the left side are < piv, all elements on the
  // right are >= piv
  delete reserved;

  // Element "between" final_left and final_right might not have
  // been regarded yet
  if (final_left < n && !__pred(b[final_left]))
    // Really swapped.
    return final_left;
  else
    return final_left + 1;
}

#endif
