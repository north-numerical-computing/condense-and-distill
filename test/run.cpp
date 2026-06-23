#include <vector>
#include <catch2/catch_test_macros.hpp>

#include <catch2/catch_session.hpp>

#include "cad.h"

std::vector<double> randomDataFixed(unsigned int n);

int main(int argc, char** argv)
{ int result = Catch::Session().run(argc, argv); }

TEST_CASE("random") {
  auto data = randomDataFixed(1024);

  double sum = cadSum(data);
}
