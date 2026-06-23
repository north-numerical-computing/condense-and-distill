#pragma once 

/* based on paper "Library for Double-Double and Quad-Double Arithmetic" by David H. Bailey */

#include <vector>

struct qdouble {
  double a,
         b,
         c,
         d;
};

void twoSum(double a, double b, double& sum, double& err) {
  sum = a + b;
  double v = sum - a;
  err = (a - (sum - v)) + (b - v);
}

qdouble renormalise(double a0, double a1, double a2, double a3, double a4) {
  double sum,
         t1, t2, t3, t4,
         err;

  twoSum(a3, a4,  sum, t4);
  twoSum(a2, sum, sum, t3);
  twoSum(a1, sum, sum, t2);
  twoSum(a0, sum, sum,  t1);

  qdouble out;

  twoSum(sum, t1, sum, err);
  if (err != 0) {
    out.a = sum;
    sum = err;
  }

  twoSum(sum, t2, sum, err);
  if (err != 0) {
    out.b = sum;
    sum = err;
  }

  twoSum(sum, t3, sum, err);
  if (err != 0) {
    out.c = sum;
    sum = err;
  }

  twoSum(sum, t4, sum, err);
  if (err != 0) {
    out.d = sum;
    sum = err;
  }

  return out;
}

double qdoubleSum(const std::vector<double>& sum) {
  qdouble current{0, 0, 0, 0};

  auto readPtr = sum.cbegin();

  if (readPtr != sum.cend()) current.a = *readPtr++;
  if (readPtr != sum.cend()) current.b = *readPtr++;
  if (readPtr != sum.cend()) current.c = *readPtr++;
  if (readPtr != sum.cend()) current.d = *readPtr++;

  for ( ; readPtr != sum.cend() ; readPtr++) {
    current = renormalise(current.a, current.b, current.c, current.d, *readPtr);
  }

  return current.a + current.b + current.c + current.d;
}
