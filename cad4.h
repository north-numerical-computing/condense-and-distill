#pragma once 

#include <cstdint>
#include <vector>
#include <span>

#include "qdouble1.h"

namespace cad4 {

union _ddata {
  double a;
  uint64_t b;
};

uint16_t generateKey(double x) {
  _ddata y;
  y.a = x;
  return (y.b >> 51) | (y.b & 1);
}

void insert(std::vector<double>& bucket, double value, uint16_t key, uint16_t& lowestKey, uint16_t& highestKey) {
  if (key < lowestKey) lowestKey = key;
  if (key > highestKey) highestKey = key;

  if (bucket[key] == 0) {
    bucket[key] = value;
    return;
  }

  double newValue = bucket[key] + value;
  uint16_t newKey = generateKey(newValue);

  bucket[key] = 0;

  insert(bucket, newValue, newKey, lowestKey, highestKey);
}

double cadSum(std::span<double> sum) {
  std::vector<double> bucket(8191, 0);
  double total;

  uint16_t lowestKey,
            highestKey;

  lowestKey = bucket.size();
  highestKey = 0;

  for (double i : sum) {
    uint16_t accumulator = generateKey(i);

    insert(bucket, i, accumulator, lowestKey, highestKey);
  }

  qdouble1::qdouble out = {0, 0, 0, 0};

  for (size_t i = lowestKey ; i < highestKey ; i++) {
    qdouble1::qdoubleAdd(out, bucket[i]);
  }

  return out.a + out.b + out.c + out.d;
}

}