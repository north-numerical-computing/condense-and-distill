#pragma once 

#include <iostream>
#include <cstdint>
#include <vector>
#include <span>

#include "qdouble1.h"

namespace cad5 {

union _ddata {
  double a;
  uint64_t b;
};

constexpr uint64_t MASK = 0xFFF0000000000001;

inline bool keyEquals(double a, double b) {
  _ddata da, db;

  da.a = a;
  db.a = b;

  return (da.b & MASK) == (db.b & MASK);
}

struct bucket_t {
  bucket_t() = default;

  void insert(double value) {
    _ddata dValue;
    dValue.a = value;

    auto pos = std::find_if(data.begin(), data.end(), [](_ddata d) { return d.a == 0.0; });
    if (pos != data.end()) {
      *pos = dValue; 
      return;
    }
    data.push_back(dValue);
  }

  void remove(double value) {
      auto pos = std::find_if(data.begin(), data.end(), [value](_ddata k) { return keyEquals(k.a, value); });
      pos->a = 0.0;
  }

  int contains(double value) {
    _ddata dValue;
    dValue.a = value;


    for (auto i = 0 ; i < data.size() ; i++) {
      if (keyEquals(value, data[i].a)) return i;
    }
    return -1;
  }
  std::vector<_ddata> data;
};

void insert(bucket_t& bucket, double value) {
  int index = bucket.contains(value);
  if (index == -1) {
    bucket.insert(value);
    return;
  }

  double newValue = bucket.data[index].a + value;
  
  if (keyEquals(value, newValue)) {
    bucket.data[index].a = newValue;
    return;
  }

  bucket.remove(value);

  insert(bucket, newValue);
}

double cadSum(std::span<double> sum) {
  bucket_t bucket;
  double total;

  for (double i : sum) {
    insert(bucket, i);
  }

  qdouble1::qdouble out = {0, 0, 0, 0};

  for (auto d : bucket.data) {
    out = qdouble1::qdoubleAdd(out, d.a);
  }

  return out.a + out.b + out.c + out.d;
}

}