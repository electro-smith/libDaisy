#!/bin/bash
#TODO:
# - fix all the hard-coding
STARTDIR='./src/'
find $STARTDIR -iname '*.h' -type f -exec sed -i 's/\t/    /g' {} +
find $STARTDIR -iname '*.cpp' -type f -exec sed -i 's/\t/    /g' {} +
clang-format-6.0 -i $STARTDIR/*.cpp $STARTDIR/*.h
