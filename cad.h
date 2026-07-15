#pragma once 

#include <cstdint>
#include <cstring>
#include <vector>
#include <span>
#include <memory>

#include <iostream>
#include <iomanip>
#include <cassert>

#include "qdouble.h"

union _ddata {
  double a;
  uint64_t b;
};

inline uint16_t generateKey(double x) {
  _ddata y;
  y.a = x;
  return (std::rotr(y.b, 1) >> 51);
}

void insert(std::vector<double>& bucket, double value, uint16_t key) {
  _ddata x;

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

double cadSum(std::span<double> sum) {
  std::vector<double> bucket(8191, 0);
  double total;

  for (double i : sum) {
    uint16_t accumulator = generateKey(i);

    insert(bucket, i, accumulator);
  }

  #ifndef NDEBUG
  assertKeys(bucket);
  #endif

  return qdoubleSum(bucket);
}
