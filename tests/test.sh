#!/usr/bin/env bash

MORLOC_REPO=$1

mkdir -p src
cp -r "${MORLOC_REPO}/data/lang" src
cp "${MORLOC_REPO}/data/morloc.h" src/morloc.h

mkdir -p lib
g++ -g -o cpptest -Isrc -Isrc/lang/cpp -Isrc/lang "test.cpp"

# # echo ""
# echo "Testing R"
# Rscript "test.R"

# echo ""
# echo "Testing python"
# ln -sf $PWD/../lang/py/pympack.cpython-312-x86_64-linux-gnu.so $PWD/pympack.cpython-312-x86_64-linux-gnu.so

# python "test.py"
