#include <cmath>
#include <vector>

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

TEST_CASE("powers of two") {
  using namespace Catch::Matchers;

  const int repeat = 1;
  const int k = -1;
  const double expected = std::pow(2, k) * repeat;

  auto data = powersOfTwo(repeat, k);

  double sum = cadSum(data);
  double result = std::abs(expected - sum) / std::abs(expected);
  REQUIRE_THAT(result, WithinRel(0, 0.000001));
}

static std::vector<double> absVector(const std::vector<double>& data) {
  std::vector<double> out;

  out.reserve(data.size());

  for (double x : data) {
    out.push_back(std::abs(x));
  }

  return out;
}

TEST_CASE("e^-x maclaurin series") {
  using namespace Catch::Matchers;

  const int exponent = -10;
  const int n = 200;
  auto data = expMaclaurin(exponent, n);
  auto absData = absVector(data);

  double sum = cadSum(data);

  /*
  double absSum = cadSum(absData);
  
  double condition = absSum / std::abs(sum);

  std::cout << condition / std::pow(2, 53) << "\n";
  REQUIRE_THAT(condition, WithinAbs(0, 0.00001));
  
  std::cout << std::exp(exponent) << ", " << sum << "\n";
  */
  REQUIRE_THAT(sum, WithinAbs(std::exp(exponent), 0.00001));
}

TEST_CASE("correct key generation") {
  using namespace Catch::Matchers;

  REQUIRE(generateKey(2.0) == 2048);
  REQUIRE(generateKey(3.0) == 2048);
  REQUIRE(generateKey(2.0000000000000004) == 2049);
  REQUIRE(generateKey(3.0000000000000004) == 2049);
}