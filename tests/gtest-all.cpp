// The .cpp => .o rule relies on the cpp files being in the exact same folder hierarchy as the .o files in the build directory.
// I'm no makefile guru - how can we add files in ../googletest/src to the makefile directly without this ugly include here?!
#include "googletest/src/gtest-all.cc"