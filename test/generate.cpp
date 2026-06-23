/* functions for generating test data */

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
