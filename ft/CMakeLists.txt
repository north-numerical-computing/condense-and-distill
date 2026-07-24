cmake_minimum_required(VERSION 3.20)

project(ft-modules LANGUAGES C CXX Fortran)

include(FortranCInterface)

set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -lgfortran -fopenmp -fallow-argument-mismatch")

add_library(ft-modules AccSum.f90 DemmelHidaSum.f90 ExactSum.f90 FastAccSum.f90 PrecSum.f90)
