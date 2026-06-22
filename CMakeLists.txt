cmake_minimum_required(VERSION 3.30)

project(cad LANGUAGES CXX)

add_subdirectory(test)

file(CREATE_LINK
  "${CMAKE_BINARY_DIR}/compile_commands.json"
  "${CMAKE_SOURCE_DIR}/compile_commands.json"
  SYMBOLIC
)

enable_testing()
