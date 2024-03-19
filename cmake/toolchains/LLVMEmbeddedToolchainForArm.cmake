# adapted from https://github.com/jobroe/cmake-arm-embedded/blob/master/toolchain-arm-none-eabi.cmake

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_LIST_DIR}")

set(TOOLCHAIN arm-none-eabi)

if(NOT DEFINED CMAKE_C_COMPILER)
    if(NOT DEFINED ENV{CMAKE_C_COMPILER})
        message(FATAL_ERROR "Please provide CMAKE_C_COMPILER at configuration if not using autodetection")
    endif()

    set(CMAKE_C_COMPILER $ENV{CMAKE_C_COMPILER})
endif()

cmake_path(GET CMAKE_C_COMPILER PARENT_PATH TOOLCHAIN_BIN_DIR)
cmake_path(GET TOOLCHAIN_BIN_DIR PARENT_PATH TOOLCHAIN_PREFIX)

if(WIN32)
    set(TOOLCHAIN_EXT ".exe")
else()
    set(TOOLCHAIN_EXT "")
endif()

include(stm32h750xx)

set(CMAKE_ASM_FLAGS "-x assembler-with-cpp" CACHE INTERNAL "ASM Compiler options")

# ---------------------------------------------------------------------------------------
# Set debug/release build configuration Options
# ---------------------------------------------------------------------------------------

# Options for DEBUG build
set(CMAKE_C_FLAGS_DEBUG "-fno-finite-loops" CACHE INTERNAL "C Compiler options for debug build type")
set(CMAKE_CXX_FLAGS_DEBUG "-fno-finite-loops" CACHE INTERNAL "C++ Compiler options for debug build type")
set(CMAKE_ASM_FLAGS_DEBUG "" CACHE INTERNAL "ASM Compiler options for debug build type")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "Linker options for debug build type")

# Options for RELEASE build
set(CMAKE_C_FLAGS_RELEASE "-fno-finite-loops" CACHE INTERNAL "C Compiler options for release build type")
set(CMAKE_CXX_FLAGS_RELEASE "-fno-finite-loops" CACHE INTERNAL "C++ Compiler options for release build type")
set(CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "ASM Compiler options for release build type")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "" CACHE INTERNAL "Linker options for release build type")

# ---------------------------------------------------------------------------------------
# Set compilers
# ---------------------------------------------------------------------------------------
set(CLANG_TARGET_TRIPLE thumbv7em-none-eabihf)
set(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_DIR}/clang${TOOLCHAIN_EXT} CACHE FILEPATH "C Compiler")
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_DIR}/clang++${TOOLCHAIN_EXT} CACHE FILEPATH "C++ Compiler")
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_BIN_DIR}/clang${TOOLCHAIN_EXT} CACHE FILEPATH "ASM Compiler")

set(CMAKE_AR ${TOOLCHAIN_BIN_DIR}/llvm-ar${TOOLCHAIN_EXT} CACHE FILEPATH "Path to archiver.")
set(CMAKE_LINKER ${TOOLCHAIN_BIN_DIR}/llvm-ld${TOOLCHAIN_EXT} CACHE FILEPATH "Path to linker.")
set(CMAKE_NM ${TOOLCHAIN_BIN_DIR}/llvm-nm${TOOLCHAIN_EXT} CACHE FILEPATH "Path to list symbols.")
set(CMAKE_OBJCOPY ${TOOLCHAIN_BIN_DIR}/llvm-objcopy${TOOLCHAIN_EXT} CACHE FILEPATH "Path to objcopy.")
set(CMAKE_OBJDUMP ${TOOLCHAIN_BIN_DIR}/llvm-objdump${TOOLCHAIN_EXT} CACHE FILEPATH "Path to dump objects.")
set(CMAKE_RANLIB ${TOOLCHAIN_BIN_DIR}/llvm-ranlib${TOOLCHAIN_EXT} CACHE FILEPATH "Path to ranlib.")
set(CMAKE_SIZE ${TOOLCHAIN_BIN_DIR}/llvm-size${TOOLCHAIN_EXT} CACHE FILEPATH "Path to size.")
set(CMAKE_STRIP ${TOOLCHAIN_BIN_DIR}/llvm-strip${TOOLCHAIN_EXT} CACHE FILEPATH "Path to strip.")

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

include_directories(SYSTEM ${TOOLCHAIN_PREFIX}/lib/clang-runtimes/${TOOLCHAIN}/${ARCH_COMBO}/include/c++/v1)
