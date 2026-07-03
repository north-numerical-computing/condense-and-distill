/* functions for generating test data */

#include <algorithm>
#include <cmath>
#include <numbers>
#include <random>
#include <vector>

/* random data, fixed seed */
std::vector<double> randomDataFixed(unsigned int n) {
  std::vector<double> out;

  std::mt19937 gen(0);
  std::uniform_real_distribution<> dis(-10000, 10000);

  for (auto i = 0 ; i < n ; i++) {
    out.push_back(dis(gen));
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

/* generates range of powers of two that are both positive and negative. 
a single value (k) will be skipped on the negative side.
These powers of two can be repeated n times
resulting in a summation that should equal n*2^k
*/
std::vector<double> powersOfTwo(unsigned int repeat, int k) {
  std::vector<double> out;

  const int lower = -100;
  const int upper = 100;
  
  for (int i = lower ; i <= upper ; i++) {
    double u = std::pow(2, i);
    out.push_back(u);

    if (i == k) continue;
    out.push_back(-u);
  }

  for (int i = 1 ; i < repeat ; i++) {
    out.insert(out.end(), out.begin(), out.end());
  }

  std::mt19937 gen(0);
  std::shuffle(out.begin(), out.end(), gen);
  return out;
}
