#!/usr/bin/env bash


echo ""
echo "Testing C++"
# # I'm not sure why building this the proper way fails:
# g++ -o cpptest -I$HOME/.morloc/include -I$PWD/../lang/cpp  -L$HOME/.morloc/lib -lmlcmpack "test.cpp"
# # so instead I manually pull all the files together and build them in a temp dir
mkdir -p cppbuild
cp ../lang/cpp/cppmpack.hpp ../src/*.[ch] "test.cpp" cppbuild
cd cppbuild
g++ -std=c++20 -o ../cpptest *.[ch] *.hpp *.cpp
cd ..
./cpptest
rm -f cpptest
rm -rf cppbuild

# echo ""
# echo "Testing R"
# Rscript "test.R"
#
# echo ""
# echo "Testing python"
# ln -sf $PWD/../lang/py/pympack.py $PWD/pympack.py
# python "test.py"
