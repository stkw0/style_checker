#!/bin/sh

clang++ main.cpp -o stylechecker -I/usr/lib/llvm-3.8/include -std=c++14 -lclang -O0 -ggdb
