#!/usr/bin/env bash


echo ""
echo "Testing C++"
g++ -g -o cpptest -I$HOME/.morloc/include -I$PWD/../lang/cpp -L$HOME/.morloc/lib -lmlcmpack -Wl,-rpath,/home/z/.morloc/lib -lmlcmpack "test.cpp"
./cpptest

# echo ""
echo "Testing R"
Rscript "test.R"

echo ""
echo "Testing python"
ln -sf $PWD/../lang/py/pympack.cpython-312-x86_64-linux-gnu.so $PWD/pympack.cpython-312-x86_64-linux-gnu.so

python "test.py" 2> z-py
