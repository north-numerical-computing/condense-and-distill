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