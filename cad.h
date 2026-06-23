#pragma once 

#include <cstdint>
#include <vector>
#include <iostream>

#include "qdouble.h"

union _ddata {
  double a;
  uint64_t b;
};

uint16_t generateAccumulator(double x) {
  _ddata y;
  y.a = x;
  return (y.b >> 51) | (y.b & 1);
}

void insert(std::vector<double>& bucket, double value, uint16_t key) {
  if (bucket[key] == 0) {
    bucket[key] = value;
    return;
  }

  double newValue = bucket[key] + value;
  bucket[key] = 0;
  uint16_t newKey = generateAccumulator(newValue);
  insert(bucket, newValue, newKey);
}

double cadSum(const std::vector<double>& sum) {
  std::vector<double> bucket(8191, 0);
  double total;

  for (double i : sum) {
    uint16_t accumulator = generateAccumulator(i);

    insert(bucket, i, accumulator);
  }


  return qdoubleSum(bucket);
}
