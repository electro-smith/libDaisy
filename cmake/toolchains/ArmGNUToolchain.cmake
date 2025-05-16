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

# Common flags in OBJECT_GEN_FLAGS, architecture flags in MCU
include(stm32h750xx)

set(CMAKE_ASM_FLAGS "-x assembler-with-cpp" CACHE INTERNAL "ASM Compiler options")

# Ensure the ar plugin is loaded (needed for LTO)
set(CMAKE_AR ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-gcc-ar${TOOLCHAIN_EXT} CACHE INTERNAL "Archiving tool")
set(CMAKE_RANLIB ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-gcc-ranlib${TOOLCHAIN_EXT} CACHE INTERNAL "Indexing tool")
set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> <TARGET>")
set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> <TARGET>")

# -Wl,--gc-sections     Perform the dead code elimination.
# --specs=nano.specs    Link with newlib-nano.
# --specs=nosys.specs   No syscalls, provide empty implementations for the POSIX system calls.
set(CMAKE_EXE_LINKER_FLAGS "--specs=nano.specs --specs=nosys.specs" CACHE INTERNAL "Linker options")

# ---------------------------------------------------------------------------------------
# Set debug/release build configuration Options
# ---------------------------------------------------------------------------------------

# Options for DEBUG build
set(CMAKE_C_FLAGS_DEBUG "" CACHE INTERNAL "C Compiler options for debug build type")
set(CMAKE_CXX_FLAGS_DEBUG "" CACHE INTERNAL "C++ Compiler options for debug build type")
set(CMAKE_ASM_FLAGS_DEBUG "" CACHE INTERNAL "ASM Compiler options for debug build type")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "Linker options for debug build type")

# Options for RELEASE build
set(CMAKE_C_FLAGS_RELEASE "" CACHE INTERNAL "C Compiler options for release build type")
set(CMAKE_CXX_FLAGS_RELEASE "" CACHE INTERNAL "C++ Compiler options for release build type")
set(CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "ASM Compiler options for release build type")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "" CACHE INTERNAL "Linker options for release build type")

# ---------------------------------------------------------------------------------------
# Set compilers
# ---------------------------------------------------------------------------------------
set(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-gcc${TOOLCHAIN_EXT} CACHE INTERNAL "C Compiler")
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-g++${TOOLCHAIN_EXT} CACHE INTERNAL "C++ Compiler")
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-gcc${TOOLCHAIN_EXT} CACHE INTERNAL "ASM Compiler")

set(CMAKE_LINKER ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-ld${TOOLCHAIN_EXT} CACHE FILEPATH "Path to linker.")
set(CMAKE_NM ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-nm${TOOLCHAIN_EXT} CACHE FILEPATH "Path to list symbols.")
set(CMAKE_OBJCOPY ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-objcopy${TOOLCHAIN_EXT} CACHE FILEPATH "Path to objcopy.")
set(CMAKE_OBJDUMP ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-objdump${TOOLCHAIN_EXT} CACHE FILEPATH "Path to dump objects.")
set(CMAKE_RANLIB ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-ranlib${TOOLCHAIN_EXT} CACHE FILEPATH "Path to ranlib.")
set(CMAKE_SIZE ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-size${TOOLCHAIN_EXT} CACHE FILEPATH "Path to size.")
set(CMAKE_STRIP ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-strip${TOOLCHAIN_EXT} CACHE FILEPATH "Path to strip.")

set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PREFIX}/${${TOOLCHAIN}} ${CMAKE_PREFIX_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
