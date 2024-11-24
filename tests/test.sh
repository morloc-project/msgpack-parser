#!/usr/bin/env bash


# echo ""
# echo "Testing C++"
# ##### To build locally:
# # mkdir -p cppbuild
# # cp ../lang/cpp/cppmpack.hpp ../src/*.[ch] "test.cpp" cppbuild
# # cd cppbuild
# # g++ -g -o ../cpptest *.[ch] *.hpp *.cpp
# # cd ..
# #
# g++ -g -o cpptest -I$HOME/.morloc/include -I$PWD/../lang/cpp -L$HOME/.morloc/lib -lmlcmpack -Wl,-rpath,/home/z/.morloc/lib -lmlcmpack "test.cpp"
# ./cpptest

# echo ""
# echo "Testing R"
# Rscript "test.R"

echo ""
echo "Testing python"
ln -sf $PWD/../lang/py/pympack.py $PWD/pympack.py
python "test.py"
