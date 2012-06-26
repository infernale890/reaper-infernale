#!/bin/sh
mkdir build
cp ../conf/*.cl build/
cp ../conf/*.conf build/
cd build
cmake -D CMAKE_BUILD_TYPE=Release ..
make
