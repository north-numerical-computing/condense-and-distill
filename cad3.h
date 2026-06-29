#pragma once 

#include <cstdint>
#include <vector>
#include <span>

#include <unordered_map>

#include "qdouble1.h"

namespace cad3 {

union _ddata {
  double a;
  uint64_t b;
};

uint16_t generateKey(double x) {
  _ddata y;
  y.a = x;
  return (y.b >> 51) | (y.b & 1);
}

void insert(std::unordered_map<uint16_t, double>& bucket, double value, uint16_t key) {
  if (bucket[key] == 0) {
    bucket[key] = value;
    return;
  }

  double newValue = bucket[key] + value;
  uint16_t newKey = generateKey(newValue);

  bucket.erase(key);

  insert(bucket, newValue, newKey);
}

double cadSum(std::span<double> sum) {
  std::unordered_map<uint16_t, double> bucket;
  double total;

  for (double i : sum) {
    uint16_t accumulator = generateKey(i);

    insert(bucket, i, accumulator);
  }

  qdouble1::qdouble out = {0, 0, 0, 0};

  for (auto& value : bucket) {
    qdouble1::qdoubleAdd(out, value.second);
  }

  return out.a + out.b + out.c + out.d;
}

}