cmake_minimum_required (VERSION 3.0)

project(crcx VERSION 0.2 LANGUAGES C CXX)

add_subdirectory (src)

enable_testing()
add_subdirectory (test)
