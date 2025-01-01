#!/usr/bin/env bash

set -eu

MORLOC_REPO=$1

rm -rf src
mkdir -p src
cp -r "${MORLOC_REPO}/data/lang" src
cp "${MORLOC_REPO}/data/morloc.h" src/morloc.h
mkdir -p lib

echo "Build C++"
g++ -g -o cpptest -Isrc -Isrc/lang/cpp "test.cpp"
echo "Testing C++"
./cpptest
g++ -g -o cppbench -Isrc -Isrc/lang/cpp "bench.cpp"
g++ -O3 -o cppbench -Isrc -Isrc/lang/cpp "bench.cpp"
echo "Benchmarking C++"
./cppbench

echo "Building R"
mkdir -p rbuild
cp src/morloc.h src/lang/r/rmorloc.c rbuild
cd rbuild
R CMD SHLIB -o ../lib/librmorloc.so rmorloc.c
cd ..
# rm -rf rbuild
echo "Testing R"
Rscript "test.R"

echo ""
echo "Building python"
# reset sources to local
sed -i 's;...morloc.include;../..;' src/lang/py/setup.py
sed -i 's;...morloc.bin;../../..;' src/lang/py/setup.py
make -C src/lang/py
cp -r "${MORLOC_REPO}/data/lang/py/pymorloc.c" src/lang/py # for gdb
cp -f src/lang/py/*.so .
echo "Testing python"
python "test.py"
