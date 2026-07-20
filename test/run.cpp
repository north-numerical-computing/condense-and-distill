#include <cmath>
#include <vector>

#include <iostream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/catch_session.hpp>

#include "generate.h"

#include "cad.h"
#include "kahan.h"

int main(int argc, char** argv)
{ int result = Catch::Session().run(argc, argv); }

TEST_CASE("random") {
  using namespace Catch::Matchers;

  auto data = randomDataFixed(4096);

  double sum1 = cadSum(data);
  double sum2 = qdoubleSum(data);
  REQUIRE_THAT(sum1, WithinRel(sum2, 0.0001));
}

TEST_CASE("pythagoras") {
  using namespace Catch::Matchers;

  const unsigned int pairs = 2048;
  auto data = pythagoreanPairs(pairs);

  double sum = cadSum(data);

  REQUIRE_THAT(sum, WithinRel(pairs, 0.000001));
}

TEST_CASE("random ill-conditioned sum") {
  using namespace Catch::Matchers;

  const int repeat = 8;
  const int k = 4096;
  const int e = 32;
  const double cond = std::pow(10, 50);
  const double expected = std::pow(10, e) / cond;

  auto data = randomIllConditionedSum(repeat, k, e, cond);

  double sum = cadSum(data);
  std::cout << "ILL CONDITIONED TEST: " << expected << ", " << sum << "\n";
  REQUIRE(sum == expected); 
}

TEST_CASE("powers of two") {
  using namespace Catch::Matchers;

  const int repeat = 64;
  const int k = -1;
  const double expected = std::pow(2, k) * repeat;

  auto data = powersOfTwo(repeat, k);

  double sum = cadSum(data);
  double result = std::abs(expected - sum) / std::abs(expected);
  std::cout << "POWERS OF TWO TEST: " << sum << ", " << expected << ", " << result << "\n";
  REQUIRE(result == 0);
}

TEST_CASE("correct key generation") {
  using namespace Catch::Matchers;

  std::vector<double> out;

  double num1 = 0x1.8p+1;
  double num2 = 0x1.8000000000001p+1;
  double negation = -6;
  out.insert(out.end(), { num1, num2, negation });

  double sum = cadSum(out);

  REQUIRE(sum != 0.0);
}