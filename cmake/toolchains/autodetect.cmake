# Try to find/detect a valid embedded toolchain

if(NOT DEFINED CMAKE_C_COMPILER)
  message(NOTICE "No compiler specified. Falling back to searching PATH...")
  find_program(CMAKE_C_COMPILER "arm-none-eabi-gcc")
  find_program(CMAKE_CXX_COMPILER "arm-none-eabi-g++")

  if(${CMAKE_C_COMPILER} STREQUAL CMAKE_C_COMPILER-NOTFOUND)
    message(NOTICE "Could not find arm-none-eabi-gcc. Assuming clang is part of embedded toolchain...")
    find_program(CMAKE_C_COMPILER "clang" REQUIRED)
    find_program(CMAKE_CXX_COMPILER "clang++" REQUIRED)
  else()
    message(STATUS "Found ${CMAKE_C_COMPILER}")
  endif()
endif()

cmake_path(GET CMAKE_C_COMPILER STEM CMAKE_C_COMPILER_STEM)
cmake_path(GET CMAKE_C_COMPILER PARENT_PATH CMAKE_C_COMPILER_BIN)
cmake_path(GET CMAKE_C_COMPILER_BIN PARENT_PATH TOOLCHAIN_PREFIX)

set(CMAKE_C_COMPILER ${CMAKE_C_COMPILER} CACHE INTERNAL "Path to the C compiler")

# Used to propagate the discovered/set compiler to the ABI tests
set(ENV{CMAKE_C_COMPILER} ${CMAKE_C_COMPILER})

if(${CMAKE_C_COMPILER_STEM} STREQUAL arm-none-eabi-gcc)
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/ArmGNUToolchain.cmake)
elseif(${CMAKE_C_COMPILER_STEM} STREQUAL clang)
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/LLVMEmbeddedToolchainForArm.cmake)
endif()
