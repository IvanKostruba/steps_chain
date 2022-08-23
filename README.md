# steps_chain

## Description

A library building block that allows functions, functors and lambdas to be combined together in a sequence, so that previous function output is passed as the next function input. On each step, current argument can be retrieved in serialized form. Also, sequence can be started (or resumed) from any step, either using current state, or initializing state from serialized argument.

See examples.cpp for usage examples.

## Usage

This is a header-only library, so you can just copy it into your project.

Alternatively, you can clone the repo and run:

> mkdir build && cd build
> cmake .. && cmake --install .

And then in your CMakeLists.txt

find_package(steps_chain CONFIG REQUIRED)
target_link_libraries(yourBinary PRIVATE steps_chain)
