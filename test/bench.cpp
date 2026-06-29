#include <iostream>
#include <algorithm>
#include <chrono>
#include <functional>
#include <numeric>
#include <vector>
#include <fstream>

#include "cad0.h"
#include "cad1.h"
#include "cad2.h"
#include "cad3.h"
#include "cad4.h"
#include "cad5.h"

#include "qdouble0.h"
#include "qdouble1.h"

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
  const char* notes;
  Bench benchmark;
};

int main(int argc, char** argv) {
  std::ofstream outFile("data.csv");

  const std::vector<test_t> tests = {
    {"Quad double V1",
      "Initial implementation of slow summation algorithm",
      Bench{qdouble0::qdoubleSum, randomDataFixed}},

    {"Condense and distill V1",
      "Initial implementation of condense and distill algorithm",
      Bench{cad0::cadSum, randomDataFixed}},

    {"Quad double V2", 
      "Optimised version of quad double algorithm that ignores when zero is added, and uses the fast 2sum algorithm instead of the normal one",
      Bench{qdouble1::qdoubleSum, randomDataFixed}},

    {"Condense and distill V2",
      "Version that uses the optimised Quad double V2 for the distill stage of the algorithm",
      Bench{cad1::cadSum, randomDataFixed}},

      /*

    {"Condense and distill V3",
      "Version using std::map for the bucket",
      Bench{cad2::cadSum, randomDataFixed}},

    {"Condense and distill V4",
      "Version using std::unordered_map for the bucket",
      Bench{cad3::cadSum, randomDataFixed}},

      */

    {"Condense and distill V5",
      "Only start adding up the values from the lowest non-zero value in the bucket",
      Bench{cad4::cadSum, randomDataFixed}},

    {"Condense and distill V6",
      "Create an array that grows to add new keys",
      Bench{cad5::cadSum, randomDataFixed}}
  };

  for (auto& test : tests) {
    outFile << test.name << ",,,";
  }
  outFile << "\n";

  for (auto& test : tests) {
    outFile << test.notes << ",,,";
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
