#!/usr/bin/env bash

echo "Running test.c"
gcc test.c mlcmpack.[ch] mpack.[ch] && ./a.out

echo ""
echo "Running test-pack.c"
gcc test-pack.c mlcmpack.[ch] mpack.[ch] && ./a.out

echo ""
echo "Running parser.c"
gcc parser.c mlcmpack.[ch] mpack.[ch] && ./a.out
