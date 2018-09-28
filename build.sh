#!/bin/bash
mkdir -p build
cd build
CXX=g++-8 cmake -DCMAKE_BUILD_TYPE=Debug ../src
make
./ComputeUnit
