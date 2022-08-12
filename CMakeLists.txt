cmake_minimum_required(VERSION 3.14)

# Options. Turn on with 'cmake -Dmyvarname=ON'.
option(test "Build all tests." OFF) # Makes boolean 'test' available.

project(StepsChain)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(example)
add_subdirectory(test)