#include <cmath>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <catch2/catch_session.hpp>

#include "cad.h"
#include "kahan.h"

std::vector<double> randomDataFixed(unsigned int n);
std::vector<double> pythagoreanPairs(unsigned int n);
std::vector<double> powersOfTwo(unsigned int repeat, int k);
std::vector<double> expMaclaurin(int exponent, unsigned int n);

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

  const int repeat = 10;
  const int k = -1;
  const double expected = std::pow(2, k) * repeat;

  auto data = powersOfTwo(repeat, k);

  double sum = cadSum(data);

  REQUIRE_THAT(sum, WithinRel(expected, 0.000001));
}

TEST_CASE("e^-x maclaurin series") {
  using namespace Catch::Matchers;

  const int exponent = -30;
  const int n = 200;
  auto data = expMaclaurin(exponent, n);

  double sum = cadSum(data);

  REQUIRE_THAT(sum, WithinAbs(0, 0.00001));
}