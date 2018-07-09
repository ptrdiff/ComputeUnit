#!/bin/bash
mkdir build
cd build
CXX=g++-8 cmake ..
make
./ComputeUnit