#pragma once 

#include <cstdint>
#include <cstring>
#include <vector>
#include <span>
#include <memory>

#include <iostream>
#include <iomanip>
#include <cassert>

#ifdef __SSE2__
#include <immintrin.h>
#else 
#warning SSE2 not supported!
#endif 

#include "qdouble.h"

union _ddata {
  double a;
  uint64_t b;
};

inline uint16_t generateKey(double x) {
  _ddata y;
  y.a = x;
  return ((y.b >> 52) << 1) | (y.b & 1);
}

void insert(std::vector<double>& bucket, double value, uint16_t key) {
  for (;;) {
    if (bucket[key] == 0) {
      bucket[key] = value;
      return;
    }

    value = bucket[key] + value;

    bucket[key] = 0.0;

    key = generateKey(value);
  }
}

void assertKeys(std::vector<double>& bucket) {
  for (size_t i = 0 ; i < bucket.size() ; i++) {
    if (bucket[i] == 0.0) continue;

    if (generateKey(bucket[i]) != i) {
      assert(0);
    }
  }
}

inline __m128i generateKeys(__m128d values) {
  __m128i a = _mm_castpd_si128(values);       /* get raw bits of the two doubles */
  __m128i b = _mm_srli_epi64(a, 52);          /* shift them both down by 52 */
  b = _mm_slli_epi64(b, 1);                   /* shift them both up by 1 */
  __m128i mask = _mm_set1_epi64x(0x1);        /* create a mask for the bottom bit of both values */
  a = _mm_and_si128(a, mask);                 /* and them to get the LSB of the two doubles */
  return _mm_or_si128(a, b);                  /* return the LSB ored with the shifted values */
}

inline bool keysEqual(__m128i keys) {
  return keys[0] == keys[1];
}

std::vector<double> condense(std::span<double> input) {
  /* some issues with the final values in the array either not 
   * getting counted properly... or the readPtr extending past
   * the end of the array and adding garbage */

  std::vector<double> out(8191, 0);

  double* readPtr = input.data();
  __m128d currentOperand = _mm_load_pd(readPtr);
  readPtr += 2;

  while (readPtr <= input.data() + input.size()) {
    __m128i keys = generateKeys(currentOperand);

    if (keysEqual(keys)) {
      currentOperand[1] = currentOperand[0] + currentOperand[1];
      currentOperand[0] = *readPtr++;
      continue;
    }

    __m128d existing = _mm_set_pd(out[keys[1]], out[keys[0]]);
    currentOperand = _mm_add_pd(currentOperand, existing);

    if (out[keys[0]] == 0) {
      out[keys[0]] = currentOperand[0];
      currentOperand[0] = *readPtr++;
    }
    else {
      out[keys[0]] = 0;
    }

    if (out[keys[1]] == 0) {
      out[keys[1]] = currentOperand[1];
      if (readPtr == input.data() + input.size()) currentOperand[1] = 0;
      else currentOperand[1] = *readPtr++;
    }
    else {
      out[keys[1]] = 0;
    }
  }

  return out;
}

double cadSum(std::span<double> sum) {
  /*
  std::vector<double> bucket(8191, 0);
  double total;

  for (double i : sum) {
    uint16_t accumulator = generateKey(i);

    insert(bucket, i, accumulator);
  }
    */

  auto out = condense(sum);

  #ifndef NDEBUG
  assertKeys(out);
  #endif

  return qdoubleSum(out);
}
