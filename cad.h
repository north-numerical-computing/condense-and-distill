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

void insert(std::vector<double>& bucket, double value, uint16_t key) {
  if (bucket[key] == 0) {
    bucket[key] = value;
    return;
  }

  double newValue = bucket[key] + value;
  uint16_t newKey = generateKey(newValue);

  bucket[key] = 0;

  insert(bucket, newValue, newKey);
}

int bucketFirst;
int bucketLast;
int bucketMiddle;
int bucketCount;

void count(const std::vector<double>& data) {
  bucketFirst = -1;
  bucketLast = 0;
  bucketMiddle = 0;
  bucketCount = 0;

  for (auto i = 0 ; i < data.size() ; i++) {
      if (data[i] != 0) {
        bucketCount++;
        if (bucketFirst == -1) bucketFirst = i;
        bucketLast = i;
      }
  }

  int c = 0;

  for (auto i = 0 ; i < data.size() ; i++) {
      if (data[i] != 0) {
        if (c == (bucketCount >> 1)) {
          bucketMiddle = i;
          return;
        }
        c++;
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

  count(bucket);

  return qdoubleSum(bucket);
}
