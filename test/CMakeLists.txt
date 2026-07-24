cmake_minimum_required(VERSION 3.20)

project(cad)

include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.15.1
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests
  run.cpp
  generate.cpp
)

target_link_libraries(tests PRIVATE Catch2WithMain)
target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR})

include(Catch)

catch_discover_tests(tests)

add_executable(bench 
  bench.cpp 
  generate.cpp
)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgfortran")

target_link_libraries(bench PRIVATE ft-modules)
target_include_directories(bench PRIVATE ${CMAKE_SOURCE_DIR})
