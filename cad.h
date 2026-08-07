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
  __m256i a = _mm256_castpd_si256(values);       /* get raw bits of the four doubles */
  __m256i b = _mm256_srli_epi64(a, 52);          /* shift them all down by 52 */
  b = _mm256_slli_epi64(b, 1);                   /* shift them all up by 1 */
  __m256i mask = _mm256_set1_epi64x(0x1);        /* create a mask for the bottom bit of all values */
  a = _mm256_and_si256(a, mask);                 /* and them to get the LSB of the four doubles */
  return _mm256_or_si256(a, b);                  /* return the LSB ored with the shifted values */
}

/* returns index of equal key, -1 if all clear */
inline bool keysFourEquals(__m256i keys, __m256d& values, double*& readPtr) {
  for (int i = 0 ; i < 3 ; i++) {
    for (int j = i + 1 ; j < 4 ; j++) {
      if (keys[i] == keys[j]) {
        values[i] += values[j];
        values[j] = *readPtr++;
        return true;
      }
    }
  }
  return false;
}


/* returns index of equal key, -1 if all clear */
inline bool keysFourEquals2(__m256i keys, __m256d& values, double*& readPtr) {
  __m256i shiftedKeys;
  __m256i equal;
  int mask;
  /* Initial location of keys: 
   * | A | B || C | D |
   *
   * First compare in the same lane
   * | B | A || D | C | */
  shiftedKeys = _mm256_shuffle_epi32(keys, 0b01001110);
  equal       = _mm256_cmpeq_epi64(keys, shiftedKeys);
  mask = _mm256_movemask_epi8(equal);
  if (mask != 0) {
    if ((mask & 0xFF) != 0) {
      values[0] += values[1];
      values[1] = *readPtr++;
      return true;
    }
    else {
      values[2] += values[3];
      values[3] = *readPtr++;
      return true;
    }
  }

  /* Next shift across the lanes (a bit slower)
   * | D | C || B | A | */
  shiftedKeys = _mm256_permute4x64_epi64(shiftedKeys, 0b01001110);
  equal       = _mm256_cmpeq_epi64(keys, shiftedKeys);
  mask = _mm256_movemask_epi8(equal);
  if (mask != 0) {
    if ((mask & 0xFF) != 0) {
      values[0] += values[3];
      values[3] = *readPtr++;
      return true;
    }
    else {
      values[1] += values[2];
      values[2] = *readPtr++;
      return true;
    }
  }

  /* Finally swap the two on the same lane and compare
   * | C | D || A | B | */
  shiftedKeys = _mm256_shuffle_epi32(shiftedKeys, 0b01001110);
  equal       = _mm256_cmpeq_epi64(keys, shiftedKeys);
  mask = _mm256_movemask_epi8(equal);
  if (mask != 0) {
    if ((mask & 0xFF) != 0) {
      values[0] += values[2];
      values[2] = *readPtr++;
      return true; 
    }
    else {
      values[1] += values[3];
      values[3] = *readPtr++;
      return true;
    }
  }

  return false;
}

constexpr std::array<std::pair<int, int>, 9> keyPairs = {{
  {0,3}, {0,1}, {1,2}, {2,3}, {0,2}, {1,3}, {-1,-1}, {-1,-1}, {-1,-1}
}};

inline std::pair<int, int> keysFourEquals3(__m256i keys) {
  __m256i shift1 = _mm256_permute4x64_epi64(keys, 0b10010011);
  __m256i shift2 = _mm256_permute4x64_epi64(shift1, 0b10010011);

  __m256i equal1 = _mm256_cmpeq_epi64(keys, shift1);
  __m256i equal2 = _mm256_cmpeq_epi64(keys, shift2);

  int mask1 = _mm256_movemask_pd(_mm256_castsi256_pd(equal1));
  int mask2 = _mm256_movemask_pd(_mm256_castsi256_pd(equal2));

  uint8_t maskFinal = ((mask2 & 0b11) << 4) | (mask1 & 0b1111);

  return keyPairs[std::countr_zero(maskFinal)];
}

/* returns index of equal key, -1 if all clear, checks for NaN */
inline std::pair<int, int> keysFourEqualsDrain(__m256i keys, __m256d values) {
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

/* Four nans Jeremy. That's insane */
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

  while (readPtr <= (input.data() + input.size()) - 4) {
    __m256i keys = generateFourKeys(currentOperand);

    if (keysFourEquals(keys, currentOperand, readPtr)) continue;

    __m256d existing = _mm256_set_pd(out[keys[3]], out[keys[2]], out[keys[1]], out[keys[0]]);
    currentOperand = _mm256_add_pd(currentOperand, existing);

    for (int i = 0 ; i < 4 ; i++) {
      if (existing[i] == 0) {
        out[keys[i]] = currentOperand[i];
        currentOperand[i] = *readPtr++;
      }
      else {
        out[keys[i]] = 0;
      }

    }
  }
  while (!allNan256(currentOperand)) {
    __m256i keys = generateFourKeys(currentOperand);

    auto keyCollisions = keysFourEqualsDrain(keys, currentOperand);
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


/* Condense eight at a time impl */

__m512i generateEightKeys(__m512d values) {
  __m512i a = _mm512_castpd_si512(values);
  __m512i b = _mm512_ror_epi64(a, 1);
  __m512i c = _mm512_srli_epi64(b, 51);
  return c;
}

inline bool keysEightEquals(__m512i keys, __m512d& values, double*& readPtr) {
  for (int i = 0 ; i < 7 ; i++) {
    for (int j = i + 1 ; j < 8 ; j++) {
      if (keys[i] == keys[j]) {
        values[i] += values[j];
        values[j] = *readPtr++;
        return true;
      }
    }
  }
  return false;
}

inline bool keysEightEqualsDrain(__m512i keys, __m512d& values, double*& readPtr, double* inputSize) {
  for (int i = 0 ; i < 7 ; i++) {
    if (std::isnan(values[i])) continue;
    for (int j = i + 1 ; j < 8 ; j++) {
      if (keys[i] == keys[j]) {
        values[i] += values[j];
        if (readPtr >= inputSize) {
          values[j] = std::nan("");
        } else {
          values[j] = *readPtr++;
        }
        return true;
      }
    }
  }
  return false;
}

inline bool allNan512(__m512d values) {
  __mmask8 nanCheck = _mm512_cmp_pd_mask(values, values, 3);
  return (nanCheck == 0xFF);
}

std::vector<double> condenseEight(std::span<double> input) {
  std::vector<double> out(8191, 0);

  if (input.size() < 8) return condenseOne(input);

  double* readPtr = input.data();
  __m512d currentOperand = _mm512_loadu_pd(readPtr);
  readPtr += 8;

  int index;

  while (readPtr <= (input.data() + input.size()) - 8) {
    __m512i keys = generateEightKeys(currentOperand);

    if (keysEightEquals(keys, currentOperand, readPtr)) continue;

    __m512d existing = _mm512_set_pd(
      out[keys[7]],
      out[keys[6]],
      out[keys[5]],
      out[keys[4]],
      out[keys[3]],
      out[keys[2]],
      out[keys[1]],
      out[keys[0]]
    );
    currentOperand = _mm512_add_pd(currentOperand, existing);

    for (int i = 0 ; i < 8 ; i++) {
      if (existing[i] == 0) {
        out[keys[i]] = currentOperand[i];
        currentOperand[i] = *readPtr++;
      }
      else {
        out[keys[i]] = 0;
      }

    }
  }
  while (!allNan512(currentOperand)) {
    __m512i keys = generateEightKeys(currentOperand);

    if (keysEightEqualsDrain(keys, currentOperand, readPtr, (input.data() + input.size()))) continue;

    __m512d existing = _mm512_set_pd(
      out[keys[7]],
      out[keys[6]],
      out[keys[5]],
      out[keys[4]],
      out[keys[3]],
      out[keys[2]],
      out[keys[1]],
      out[keys[0]]
    );
    currentOperand = _mm512_add_pd(currentOperand, existing);

    for (int i = 0 ; i < 8 ; i++) {
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

#define condense condenseEight

double cadSum(std::span<double> sum) {
  auto out = condense(sum);
  
  #ifndef NDEBUG
  assertKeys(out);
  #endif

  return qdoubleSum(out);
}
