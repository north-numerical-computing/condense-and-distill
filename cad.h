#pragma once 

#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>
#include <span>
#include <memory>
#include <utility>

#include <iostream>
#include <iomanip>
#include <cassert>

#ifdef __SSE2__
#include <immintrin.h>
#else 
#warning SSE2 not supported!
#endif 

#ifndef __AVX__
#warning AVX not supported!
#endif

#include "qdouble.h"

union _ddata {
  double a;
  uint64_t b;
};

/* Condense one at a time impl */

inline uint16_t generateKey(double x) {
  _ddata y;
  y.a = x;
  return ((y.b >> 52) << 1) | (y.b & 1);
}

std::vector<double> condenseOne(std::span<double> input) {
  std::vector<double> out(8191, 0);
  uint16_t key;

  for (double value : input) {
    key = generateKey(value);

    for (;;) {
      if (out[key] == 0) {
        out[key] = value;
        break;
      }

      value = out[key] + value;

      out[key] = 0.0;

      key = generateKey(value);
    }
  } 
  return out;
}

/* Condense two at a time impl */

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

std::vector<double> condenseTwo(std::span<double> input) {
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

/* Condense four at a time impl */

inline __m256i generateFourKeys(__m256d values) {
  __m256i a = _mm256_castpd_si256(values);       /* get raw bits of the two doubles */
  __m256i b = _mm256_srli_epi64(a, 52);          /* shift them both down by 52 */
  b = _mm256_slli_epi64(b, 1);                   /* shift them both up by 1 */
  __m256i mask = _mm256_set1_epi64x(0x1);        /* create a mask for the bottom bit of both values */
  a = _mm256_and_si256(a, mask);                 /* and them to get the LSB of the two doubles */
  return _mm256_or_si256(a, b);                  /* return the LSB ored with the shifted values */
}

/* returns index of equal key, -1 if all clear */
inline std::pair<int, int> keysFourEquals(__m256i keys, __m256d values) {
  for (int i = 0 ; i < 3 ; i++) {
    if (std::isnan(values[i])) continue;
    for (int j = i + 1 ; j < 4 ; j++) {
      if (keys[i] == keys[j]) {
        return {i, j};
      }
    }
  }
  return {-1, -1};
}

inline bool allNan256(__m256d values) {
  __m256d nanMask = _mm256_cmp_pd(values, values, 3);
  int nanCheck = _mm256_movemask_pd(nanMask);
  return (nanCheck == 0xF);
}

std::vector<double> condenseFour(std::span<double> input) {
  std::vector<double> out(8191, 0);

  if (input.size() < 4) return condenseOne(input);

  double* readPtr = input.data();
  __m256d currentOperand = _mm256_loadu_pd(readPtr);
  readPtr += 4;

  int index;

  while (!allNan256(currentOperand)) {
    __m256i keys = generateFourKeys(currentOperand);

    auto keyCollisions = keysFourEquals(keys, currentOperand);
    if (keyCollisions.first != -1) {
      currentOperand[keyCollisions.second] = currentOperand[keyCollisions.first] + currentOperand[keyCollisions.second];
      if (readPtr >= input.data() + input.size()) {
          currentOperand[keyCollisions.first] = std::nan("");
      } else { 
        currentOperand[keyCollisions.first] = *readPtr++;
      }
      continue;
    }

    __m256d existing = _mm256_set_pd(out[keys[3]], out[keys[2]], out[keys[1]], out[keys[0]]);
    currentOperand = _mm256_add_pd(currentOperand, existing);

    for (int i = 0 ; i < 4 ; i++) {
      if (std::isnan(currentOperand[i])) continue;
      if (existing[i] == 0) {
        out[keys[i]] = currentOperand[i];
        if (readPtr >= input.data() + input.size()) {
          currentOperand[i] = std::nan("");
        } else { 
          currentOperand[i] = *readPtr++;
        }
      }
      else {
        out[keys[i]] = 0;
      }

    }
  }
  return out;
}

void assertKeys(std::vector<double>& bucket) {
  for (size_t i = 0 ; i < bucket.size() ; i++) {
    if (bucket[i] == 0.0) continue;

    if (generateKey(bucket[i]) != i) {
      assert(0);
    }
  }
}

#define condense condenseFour

double cadSum(std::span<double> sum) {
  auto out = condense(sum);
  
  #ifndef NDEBUG
  assertKeys(out);
  #endif

  return qdoubleSum(out);
}
