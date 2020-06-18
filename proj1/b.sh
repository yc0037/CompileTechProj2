#!/bin/sh

#mkdir build;
cd build;
cmake ..;
make -j 4;
cd project1
./test1
