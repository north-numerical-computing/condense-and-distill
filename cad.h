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

double cadSum(const std::span<double>& sum) {
  std::unordered_map<uint16_t, double> bucket;
  double total;

  for (double i : sum) {
    uint16_t accumulator = generateKey(i);

    insert(bucket, i, accumulator);
  }

  qdouble current{0, 0, 0, 0};

  for (auto i : bucket) {
    current = qdoubleAdd(current, i.second);
  }

  return current.a + current.b + current.c + current.d;
}
