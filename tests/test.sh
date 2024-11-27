#!/usr/bin/env bash


echo ""
echo "Testing C++"
g++ -O -g -o cpptest1 -I$HOME/.morloc/include -I$PWD/../lang/cpp -L$HOME/.morloc/lib -lmlcmpack -Wl,-rpath,/home/z/.morloc/lib -lmlcmpack "test.cpp"
g++    -g -o cpptest0 -I$HOME/.morloc/include -I$PWD/../lang/cpp -L$HOME/.morloc/lib -lmlcmpack -Wl,-rpath,/home/z/.morloc/lib -lmlcmpack "test.cpp"

echo "With no optimization"
./cpptest0

echo
echo "-O"
./cpptest1

# # echo ""
# # echo "Testing R"
# # Rscript "test.R"

echo ""
echo "Testing python"
ln -sf $PWD/../lang/py/pympack.cpython-312-x86_64-linux-gnu.so $PWD/pympack.cpython-312-x86_64-linux-gnu.so

python "test.py" 2> z-py
