# adapted from https://github.com/jobroe/cmake-arm-embedded/blob/master/toolchain-arm-none-eabi.cmake

set(CMAKE_SYSTEM_NAME  Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_LIST_DIR}")

set(TOOLCHAIN arm-none-eabi)
if(NOT DEFINED TOOLCHAIN_PREFIX)
    message(STATUS "Please specify the TOOLCHAIN_PREFIX !\n For example: -DTOOLCHAIN_PREFIX=\"path/to/LLVMEmbeddedToolchainForArm\"")
endif()
cmake_path(SET TOOLCHAIN_BIN_DIR ${TOOLCHAIN_PREFIX}/bin)

if(WIN32)
    set(TOOLCHAIN_EXT ".exe" )
else()
    set(TOOLCHAIN_EXT "" )
endif()

include(stm32h750xx)
link_libraries(semihost)

set(CMAKE_ASM_FLAGS "-x assembler-with-cpp" CACHE INTERNAL "ASM Compiler options")

#---------------------------------------------------------------------------------------
# Set debug/release build configuration Options
#---------------------------------------------------------------------------------------

# Options for DEBUG build
# -Og   Enables optimizations that do not interfere with debugging.
# -g    Produce debugging information in the operating system’s native format.
set(CMAKE_C_FLAGS_DEBUG "-O0 -g" CACHE INTERNAL "C Compiler options for debug build type")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g" CACHE INTERNAL "C++ Compiler options for debug build type")
set(CMAKE_ASM_FLAGS_DEBUG "-g" CACHE INTERNAL "ASM Compiler options for debug build type")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "Linker options for debug build type")

# Options for RELEASE build
# -Os   Optimize for size. -Os enables all -O2 optimizations.
# -flto Runs the standard link-time optimizer.
set(CMAKE_C_FLAGS_RELEASE "-Oz -flto" CACHE INTERNAL "C Compiler options for release build type")
set(CMAKE_CXX_FLAGS_RELEASE "-Oz -flto" CACHE INTERNAL "C++ Compiler options for release build type")
set(CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "ASM Compiler options for release build type")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto" CACHE INTERNAL "Linker options for release build type")


#---------------------------------------------------------------------------------------
# Set compilers
#---------------------------------------------------------------------------------------
set(CLANG_TARGET_TRIPLE arm-none-eabihf)
set(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_DIR}/clang${TOOLCHAIN_EXT} CACHE INTERNAL "C Compiler")
set(CMAKE_C_COMPILER_TARGET ${CLANG_TARGET_TRIPLE})
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_DIR}/clang++${TOOLCHAIN_EXT} CACHE INTERNAL "C++ Compiler")
set(CMAKE_CXX_COMPILER_TARGET ${CLANG_TARGET_TRIPLE})
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_BIN_DIR}/clang${TOOLCHAIN_EXT} CACHE INTERNAL "ASM Compiler")
set(CMAKE_ASM_COMPILER_TARGET ${CLANG_TARGET_TRIPLE})

set(CMAKE_SYSROOT ${TOOLCHAIN_PREFIX}/lib/clang-runtimes/${TOOLCHAIN}/${ARCH_COMBO})
set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PREFIX}/${TOOLCHAIN} ${CMAKE_PREFIX_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
