#!/bin/bash
# Uses clang-format-10.0.0
#TODO:
# - fix all the hard-coding
STARTDIR='./src/**'
find $STARTDIR -iname '*.h' -type f -exec sed -i 's/\t/    /g' {} +
find $STARTDIR -iname '*.cpp' -type f -exec sed -i 's/\t/    /g' {} +
clang-format  --verbose -i $STARTDIR.cpp $STARTDIR.c $STARTDIR.h $STARTDIR/*.cpp $STARTDIR/*.c $STARTDIR/*.h
