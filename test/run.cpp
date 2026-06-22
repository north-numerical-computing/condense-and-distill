#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>

#include "cad.h"

int main(int argc, char** argv)
{ int result = Catch::Session().run(argc, argv); }

TEST_CASE("init") {
  CHECK(cadSum({}) == 0.0);
}
