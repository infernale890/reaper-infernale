#!/bin/sh
mkdir build
cp *.cl build/
cp *.conf build/
cd build
cmake -D CMAKE_BUILD_TYPE=Release ..
make
