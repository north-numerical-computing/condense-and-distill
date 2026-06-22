cmake_minimum_required(VERSION 3.30)

project(tests)

include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.15.1
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests
  run.cpp
)

target_link_libraries(tests PRIVATE Catch2WithMain)
target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR})

include(Catch)

catch_discover_tests(tests)



