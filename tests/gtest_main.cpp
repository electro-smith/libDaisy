#include <cstdio>
#include "gtest/gtest.h"

#ifdef __CYGWIN__
#include <string>
#include <regex>
#include <vector>
std::vector<std::vector<char>> replacedStringStorage;
void replaceWinPathWithCygwinPath(int argc, char **argv)
{
    // find gtest_output argument
    for(int i = 0; i < argc; i++)
    {
        auto str = std::string(argv[i]);
        if(str.rfind("--gtest_output", 0) == 0)
        {
            // this is the path we need to modify
            // replace drive letter, then replace all backslashes
            str = std::regex_replace(
                str, std::regex("([A-Z]):\\\\"), "/cygdrive/$1/");
            str = std::regex_replace(str, std::regex("\\\\"), "/");
            // push into the storage so that it persists after this function
            replacedStringStorage.push_back({str.begin(), str.end()});
            // replace original argument
            argv[i] = replacedStringStorage.back().data();
        }
    }
}
#endif

int main(int argc, char **argv)
{
#ifdef __CYGWIN__
    // cygwin-build binaries want their paths to be cygwin-style paths.
    // This collides with google-test adapters for IDEs, which will pass
    // Windows-style paths. We need to translate these paths.
    replaceWinPathWithCygwinPath(argc, argv);
#endif
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}