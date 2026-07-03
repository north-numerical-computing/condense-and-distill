#pragma once 

#include <cstdint>
#include <array>
#include <span>
#include <thread>
#include <mutex>

#include "qdouble1.h"

namespace cad5 {

union _ddata {
  double a;
  uint64_t b;
};

inline uint16_t generateKey(double x) {
  _ddata y;
  y.a = x;
  return (y.b >> 51) | (y.b & 1);
}

void insert(std::array<double, 8191>& bucket, double value, uint16_t key) {
  if (bucket[key] == 0) {
    bucket[key] = value;
    return;
  }

  double newValue = bucket[key] + value;
  uint16_t newKey = generateKey(newValue);

  bucket[key] = 0;

  insert(bucket, newValue, newKey);
}

double cadSum(std::span<double> sum) {
  std::array<double, 8191> bucket = {0.0};
  double total;

  for (double i : sum) {
    uint16_t accumulator = generateKey(i);

    insert(bucket, i, accumulator);
  }

  return qdouble1::qdoubleSum(bucket);
}

}