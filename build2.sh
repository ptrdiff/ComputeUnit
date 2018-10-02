#!/bin/bash
mkdir -p build2
cd build2
CXX=g++-8 cmake -DCMAKE_BUILD_TYPE=Debug ../src && make && ./ComputeUnit
