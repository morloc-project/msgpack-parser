#!/usr/bin/env bash

# echo "Running test.c"
# gcc test.c mlcmpack.[ch] mpack.[ch] && ./a.out

echo ""
echo "Running test-pack.c"
gcc test-pack.c mlcmpack.[ch] mpack.[ch] && ./a.out

echo ""
echo "Testing python"
make pylib && ./mlcmpack.py
make clean

echo ""
echo "Testing C++"
make cpp && ./a.out
make clean

echo ""
echo "Testing R"
make rlib && Rscript mlcmpack.R
