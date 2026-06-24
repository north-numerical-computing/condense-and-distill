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
  double s0, s1, s2 = 0.0, s3 = 0.0;

  twoSum(a3, a4,  s0, a4);
  twoSum(a2, s0, s0, a3);
  twoSum(a1, s0, s0, a2);
  twoSum(a0, s0, a0, a1);

  s0 = a0;
  s1 = a1;

  if (s1 != 0.0) {
    twoSum(s1, a2, s1, s2);
    if (s2 != 0.0) {
      twoSum(s2, a3, s2, s3);
      if (s3 != 0.0) {
        s3 += a4;
      }
      else {
        twoSum(s2, a4, s2, s3);
      }
    }
    else {
      twoSum(s1, a3, s1, s2);
      if (s2 != 0.0) {
        twoSum(s2, a4, s2, s3);
      }
      else {
        twoSum(s1, a4, s1, s2);
      }
    }
  }
  else {
    twoSum(s0, a2, s0, s1);
    if (s1 != 0.0) {
      twoSum(s1, a3, s1, s2);
      if (s2 != 0.0) {
        twoSum(s2, a4, s2, s3);
      }
      else {
        twoSum(s1, a4, s1, s2);
      }
    }
    else {
      twoSum(s0, a3, s0, s1);
      if (s1 != 0.0) {
        twoSum(s1, a4, s1, s2);
      }
      else {
        twoSum(s0, a4, s0, s1);
      }
    }
  }

  return {s0, s1, s2, s3};
}

qdouble qdoubleAdd(qdouble a, double b) {
  double a0, a1, a2, a3;
  double e;

  twoSum(a.a, b, a0, e);
  twoSum(a.b, e, a1, e);
  twoSum(a.c, e, a2, e);
  twoSum(a.d, e, a3, e);

  return renormalise(a0, a1, a2, a3, e);
}

double qdoubleSum(const std::vector<double>& sum) {
  qdouble current{0, 0, 0, 0};

  for (double i : sum) {
    current = qdoubleAdd(current, i);
  }

  return current.a + current.b + current.c + current.d;
}
