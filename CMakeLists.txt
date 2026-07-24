cmake_minimum_required(VERSION 3.20)

project(cad LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)


add_subdirectory(test)
add_subdirectory(ft)

if (UNIX)
  file(CREATE_LINK
    "${CMAKE_BINARY_DIR}/compile_commands.json"
    "${CMAKE_SOURCE_DIR}/compile_commands.json"
    SYMBOLIC
  )
endif()

enable_testing()

