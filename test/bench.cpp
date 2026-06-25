#include <iostream>
#include <algorithm>
#include <chrono>
#include <functional>
#include <numeric>
#include <vector>
#include <fstream>

#include "cad.h"
#include "qdouble.h"

typedef std::function<double(const std::span<double>&)> summation_t;
typedef std::function<std::vector<double>(unsigned int)> data_generator_t;

const int iterations = 100;

struct statistics_t {
  double mean;
  long min,
       max;
};

struct Bench {
  Bench(summation_t algorithm, data_generator_t generator)
    : algorithm(algorithm)
    , generator(generator)
  {
  }

  auto singleTest(std::vector<double>& data) const {
    auto start = std::chrono::steady_clock::now();

    double output = algorithm(data);

    auto end = std::chrono::steady_clock::now();
    auto delta = end - start;

    return std::chrono::duration_cast<std::chrono::microseconds>(delta).count();
  }

  statistics_t operator()(unsigned int n) const {
    using namespace std::chrono_literals;

    std::vector<double> data = generator(n);

    std::vector<long> outcomes;

    for (auto i = 0 ; i < iterations ; i++) {
      outcomes.push_back(singleTest(data));
    }

    statistics_t out;
    double sum = std::accumulate(outcomes.cbegin(), outcomes.cend(), 0.0);
    
    out.mean = sum / outcomes.size();
    out.min = *std::min_element(outcomes.begin(), outcomes.end());
    out.max = *std::max_element(outcomes.begin(), outcomes.end());
    return out;
  }

  private:
  summation_t algorithm;
  data_generator_t generator;
};

std::vector<double> randomDataFixed(unsigned int n);

struct test_t {
  const char* name;
  Bench benchmark;
};

int main(int argc, char** argv) {
  std::ofstream outFile("data.csv");

  const std::vector<test_t> tests = {
    {"Condense and distill", Bench{cadSum, randomDataFixed}}
    // {"Quad double",          Bench{qdoubleSum, randomDataFixed}}
  };

  for (auto& test : tests) {
    outFile << test.name << ",,,";
  }
  outFile << "\n";

  for (auto& test : tests) {
    outFile << "mean,min,max,";
  }
  outFile << "\n";

  int lower = 1000;
  int upper = 100000;
  int step =  1000;

  for (auto i = lower; i < upper; i += step) {
    std::cout << (100 * (i - lower)) / (upper - lower) << "% \n";
    for (auto& test : tests) {
      statistics_t stats = test.benchmark(i);
      outFile << stats.mean << "," << stats.min << "," << stats.max << ",";
    }
    outFile << "\n";
  }
}
