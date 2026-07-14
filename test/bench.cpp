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
  const char* notes;
  Bench benchmark;
};

int main(int argc, char** argv) {
  std::ofstream outFile("data.json");

  const std::vector<test_t> tests = {
    {"Condense and distill", "Basic implementation of condense and distill based on original paper.", Bench{cadSum, randomDataFixed}},
    {"Quad double", "Implementation of quad-double algorithm (slow but accurate)", Bench{qdoubleSum, randomDataFixed}}
  };

  const int lower = 1000;
  const int upper = 100000;
  const int step =  1000;

  double count = 1;

  outFile << "[";

  bool testComma = false;

  for (auto& test : tests) {
    std::cout << "test " << count << "/" << tests.size() << "\n";

    if (testComma) outFile << ",";
    outFile << "{\"Name\": \"" << test.name << "\",";
    outFile << "\"Comment\": \"" << test.notes << "\",";
    outFile << "\"Data\": [";

    bool dataComma = false;

    for (auto i = lower; i <= upper; i += step) {
      if (dataComma) outFile << ",";
      statistics_t stats = test.benchmark(i);
      outFile << "[" << stats.mean << ", " << stats.min << ", " << stats.max << "]";
      dataComma = true;
    }
    outFile << "]}";
    testComma = true;
    count++;
  }

  std::cout << "complete";

  outFile << "]";
}
