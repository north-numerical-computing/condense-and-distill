#include <iostream>
#include <algorithm>
#include <chrono>
#include <functional>
#include <numeric>
#include <vector>
#include <fstream>

#include "generate.h"

#include "cad.h"
#include "qdouble.h"

typedef std::function<double(const std::span<double>&)> summation_t;
typedef std::function<std::vector<double>(unsigned int)> data_generator_t;

int iterations = 100;

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

struct test_t {
  const char* name;
  const char* notes;
  Bench benchmark;
};

struct args_t {
  std::string outPath = "data.json";
  int iterations = 100;
  bool help = false;
  std::vector<std::string> disableList;

  enum arg_state_t {
    asNew,
    asOutPath,
    asIterations,
    asDisable
  };

  arg_state_t state = asNew;
  arg_state_t nextState = state;

  args_t(int argc, char** argv) {

    for (int i = 1 ; i < argc ; i++) {
      std::string current(argv[i]);
      switch (state) {
        case (asNew): {
          if      (current == "-o") nextState = asOutPath;
          else if (current == "-i") nextState = asIterations;
          else if (current == "-d") nextState = asDisable;
          else {
            std::cout << "Unknown arg " << current << "\n";
            help = true;
            return;
          }
        }; break;
        case (asOutPath): {
          outPath = current;
          nextState = asNew;
        }; break;
        case (asIterations): {
          if (current.find_first_not_of("0123456789") != std::string::npos) {
            std::cout << "Invalid number of iterations (" << current << ")\n";
            help = true;
            return;
          }
          iterations = std::stoi(current);
          nextState = asNew;
        }; break;
        case (asDisable): {
          disableList.push_back(current);
          nextState = asNew;
        }; break;
      }

      state = nextState;
    }

    if (state != asNew) {
      std::cout << "Invalid args!\n";
      help = true;
    }
  }
};

int main(int argc, char** argv) {
  args_t args(argc, argv);

  if (args.help) {
    std::cout << "Usage: " << argv[0] << " [flags]\n";
    std::cout << "  -i (number of iterations)\n";
    std::cout << "  -o (output file path)\n";
    std::cout << "  -d (name of a test you want to disable)\n";
    return 0;
  }

  std::ofstream outFile(args.outPath);
  iterations = args.iterations;

  std::vector<test_t> tests = {
    {"Condense and distill", "", Bench{cadSum, randomDataFixed}},
    {"Quad double", "", Bench{qdoubleSum, randomDataFixed}}  };

  auto remover = std::remove_if(tests.begin(), tests.end(), 
                                [&] ( test_t& t ) {
                                  return std::find(args.disableList.begin(), args.disableList.end(), t.name) != args.disableList.end();
                                });
  
  tests.erase(remover, tests.end());

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
