#!/bin/bash
mkdir build
cd build
CXX=g++-8 cmake -DCMAKE_BUILD_TYPE=Debug ..
make
./ComputeUnit