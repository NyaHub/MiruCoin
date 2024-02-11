#!/bin/bash
mkdir -p build &&\
cd build &&\
cmake build .. &&\
make &&\
# clear &&\
./blockchain &&\
cd ../