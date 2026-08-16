#pragma once 

#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>
#include <span>
#include <memory>
#include <utility>
#include <algorithm>
#include <array>

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
  return (std::rotr(y.b, 1) >> 51);
}

inline std::array<double, 8192> condenseOne(std::span<double> input, uint64_t mask[128]) {
  std::array<double, 8192> out;

  uint16_t key;

  for (double value : input) {
    for (;;) {
      key = generateKey(value);
      const uint64_t bit = 1ULL << (key & 63);

      mask[key >> 6] ^= bit;

      if (mask[key >> 6] & bit) {
        out[key] = value;
        break;
      }

      value = out[key] + value;
    }
  } 
  return out;
}

#ifdef __SSE2__

/* Condense two at a time impl */

inline double incReadPtr(std::span<double>::iterator& current, std::span<double>& array, bool& active) {
  if (current == array.end()) {
    active = false;
    return 0;
  }
  else return *current++;
}

inline std::array<double, 8192> condenseTwo(std::span<double> input, uint64_t mask[128]) {
  std::array<double, 8192> out;

  auto readPtr = input.begin();
  bool activeA = true;
  bool activeB = true;
  uint16_t keyA;
  uint16_t keyB;
  __m128d values;
  __m128d existing;

  values = _mm_loadu_pd(&(*readPtr));
  readPtr += 2;

  while (activeA && activeB) {
    keyA = generateKey(values[0]);
    keyB = generateKey(values[1]);

    if (keyA == keyB) {
      values[0] += values[1];
      values[1] = incReadPtr(readPtr, input, activeB);
      keyB = generateKey(values[1]);
      continue;
    }

    const uint64_t bitA = 1ULL << (keyA & 63);
    const uint64_t bitB = 1ULL << (keyB & 63);
    mask[keyA >> 6] ^= bitA;
    mask[keyB >> 6] ^= bitB;

    existing = _mm_set_pd(out[keyB], out[keyA]);
    existing = _mm_add_pd(values, existing);

    if (mask[keyA >> 6] & bitA) {
      out[keyA] = values[0];
      values[0] = incReadPtr(readPtr, input, activeA);
    } else {
      values[0] = existing[0];
    }
    if (mask[keyB >> 6] & bitB) {
      out[keyB] = values[1];
      values[1] = incReadPtr(readPtr, input, activeB);
    } else {
      values[1] = existing[1];
    }
  }

  while (activeA) {
    keyA = generateKey(values[0]);

    const uint64_t bitA = 1ULL << (keyA & 63);
    mask[keyA >> 6] ^= bitA;

    existing[0] = out[keyA] + values[0];

    if (mask[keyA >> 6] & bitA) {
      out[keyA] = values[0];
      values[0] = incReadPtr(readPtr, input, activeA);
    } else {
      values[0] = existing[0];
    }
  }

  while (activeB) {
    keyB = generateKey(values[1]);

    const uint64_t bitB = 1ULL << (keyB & 63);
    mask[keyB >> 6] ^= bitB;

    existing[1] = out[keyB] + values[1];

    if (mask[keyB >> 6] & bitB) {
      out[keyB] = values[1];
      values[1] = incReadPtr(readPtr, input, activeB);
    } else {
      values[1] = existing[1];
    }
  }

  return out;
}

#endif

void assertKeys(std::span<double> bucket) {
  for (size_t i = 0 ; i < bucket.size() ; i++) {
    if (bucket[i] == 0.0) continue;

    if (generateKey(bucket[i]) != i) {
      assert(0);
    }
  }
}

#define condense condenseTwo

double cadSum(std::span<double> sum) {
  uint64_t mask[128] = { 0 };

  auto out = condense(sum, mask);
  
  #ifndef NDEBUG
  // assertKeys(out);
  #endif

  qdouble current = { 0, 0, 0, 0 };

  for (int index = 0 ; index < 128 ; index++) {
    if (!mask[index]) continue;

    for (int bit = 0 ; bit < 64 ; bit++) {
      if (!(mask[index] & (1ULL << bit))) continue;
      current = qdoubleAdd(current, out[index * 64 + bit]);
    }
  }

  return current.a + current.b + current.c + current.d;
}
