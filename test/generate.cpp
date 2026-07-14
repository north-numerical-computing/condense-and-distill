/* functions for generating test data */

#include <algorithm>
#include <cmath>
#include <numbers>
#include <random>

#include "generate.h"

union _ddata {
  double a;
  uint64_t b;
};

/* random data, fixed seed */
std::vector<double> randomDataFixed(unsigned int n) {
  std::vector<double> out;

  std::mt19937_64 gen(0);
  std::uniform_int_distribution<uint64_t> sign(0, 1);
  std::uniform_int_distribution<uint64_t> exponent(1, 0x7FE);
  std::uniform_int_distribution<uint64_t> mantissa(0, 0xfffffffffffff);

  for (auto i = 0 ; i < n ; i++) {
    _ddata d;
    
    d.b = (sign(gen) << 63) | (exponent(gen) << 62) | (mantissa(gen));
    out.push_back(d.a);
  }

  return out;
}

/* array of sin^2(x) + cos^2(x) pairings producing a known sum, randomly generated x, fixed seed */
std::vector<double> pythagoreanPairs(unsigned int n) {
  std::vector<double> out;

  std::mt19937 gen(0);
  std::uniform_real_distribution<> dis(0, 2 * std::numbers::pi);

  for (auto i = 0 ; i < n ; i++) {
    double x = dis(gen);
    double currSin = std::sin(x);
    double currCos = std::cos(x);

    out.push_back(currSin * currSin);
    out.push_back(currCos * currCos);
  }

  std::shuffle(out.begin(), out.end(), gen);

  return out;
}

/* based on 4.1 Experimental Protocol from paper "Condense and Distill" by Stef Graillat and Theo Mary.
Generates k random summands x_i - within range [10^-e, 10^e],
another k summands -x_i, and then a final summand (10^e / cond).
This gives us a known sum of (10^e / cond); its conditioning is of order k.
*/
std::vector<double> randomIllConditionedSum(unsigned int repeat, unsigned int k, int e, double cond) {
  std::vector<double> out;

  std::mt19937 gen(0);
  std::uniform_real_distribution<> dis(std::pow(10, -e), std::pow(10, e));

  for (auto i = 0; i < k; i++) {
    double temp = dis(gen);
    for (auto j = 0; j < repeat; j++) {
      out.push_back(temp);
      out.push_back(-temp);
    }
  }

  out.push_back(std::pow(10, e) / cond);

  std::shuffle(out.begin(), out.end(), gen);

  return out;
}

/* generates range of powers of two that are both positive and negative. 
a single value (k) will be skipped on the negative side.
These powers of two can be repeated n times
resulting in a summation that should equal n*2^k
*/
std::vector<double> powersOfTwo(unsigned int repeat, int k) {
  std::vector<double> temp;

  const int lower = -32;
  const int upper = 32;
  
  for (int i = lower ; i <= upper ; i++) {
    double u = std::pow(2, i);
    temp.push_back(u);

    if (i == k) continue;
    temp.push_back(-u);
  }

  std::vector<double> out;
  out.reserve(temp.size() * repeat);

  for (int i = 0 ; i < repeat ; i++) {
    out.insert(out.end(), temp.begin(), temp.end());
  }

  std::mt19937 gen(0);
  std::shuffle(out.begin(), out.end(), gen);
  return out;
}

/* generates maclaurin series for e^-x.
By setting the exponent to a negative number with large magnitude,
e^-x approximately equals 0,and therefore the sum of the generated series
should approximate 0, assuming a long enough sum is taken.
*/
std::vector<double> expMaclaurin(int exponent, unsigned int n) {
  std::vector<double> out;

  double u = 1;

  for (auto i = 1 ; i < n ; i++) {
    out.push_back(u);
    u *= (double)exponent / (double)i;
  }

  return out;
}