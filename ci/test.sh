#!/bin/bash
set -e

mkdir -p build

cd build

cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang-19 -DCMAKE_CXX_COMPILER=clang++-19

make -j 3DRendererPrototype-Tests

