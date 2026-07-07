#pragma once 

#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>
#include <span>
#include <memory>

#include <unordered_map>

#include "qdouble.h"

union _ddata {
  double a;
  uint64_t b;
};

inline uint16_t generateKey(double x) {
  _ddata y;
  y.a = x;
  return (y.b >> 51) | (y.b & 1);
}

void insert(std::vector<double>::iterator writePtr, double value, uint16_t key) {
  if (*writePtr == 0) {
    *writePtr = value;
    return;
  }

  double newValue = (*writePtr) + value;
  uint16_t newKey = generateKey(newValue);

  *writePtr = 0.0;

  if (newKey != key) writePtr++;

  insert(writePtr, newValue, newKey);
}

double cadSum(std::span<double> sum) {
  std::vector<double> bucket(8191, 0);
  double total;

  for (double i : sum) {
    uint16_t accumulator = generateKey(i);

    insert(bucket.begin() + accumulator, i, accumulator);
  }

  return qdoubleSum(bucket);
}
