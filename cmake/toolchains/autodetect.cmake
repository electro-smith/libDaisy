# Try to find/detect a valid embedded toolchain

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_LIST_DIR}")

if(NOT DEFINED CMAKE_C_COMPILER)
  message(NOTICE "No compiler specified. Falling back to searching PATH...")
  find_program(CMAKE_C_COMPILER "arm-none-eabi-gcc")
  find_program(CMAKE_CXX_COMPILER "arm-none-eabi-g++")

  if(CMAKE_C_COMPILER-NOTFOUND)
    message(NOTICE "Could not find arm-none-eabi-gcc. Assuming Clang is part of embedded toolchain...")
    find_program(CMAKE_C_COMPILER "clang" REQUIRED)
    find_program(CMAKE_CXX_COMPILER "clang++" REQUIRED)
  endif()
endif(NOT DEFINED CMAKE_C_COMPILER)

cmake_path(GET CMAKE_C_COMPILER STEM CMAKE_C_COMPILER_STEM)
cmake_path(GET CMAKE_C_COMPILER PARENT_PATH CMAKE_C_COMPILER_BIN)
cmake_path(GET CMAKE_C_COMPILER_BIN PARENT_PATH TOOLCHAIN_PREFIX)

if(${CMAKE_C_COMPILER_STEM} STREQUAL arm-none-eabi-gcc)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/ArmGNUToolchain.cmake)
elseif(${CMAKE_C_COMPILER_STEM} STREQUAL gcc)
  message(FATAL_ERROR "GCC cannot compile for arm-none-eabi. Please set CMAKE_C_COMPILER with the path to arm-none-eabi-gcc or add it to your PATH...")
elseif(${CMAKE_C_COMPILER_STEM} STREQUAL clang)
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/LLVMEmbeddedToolchainForArm.cmake)
else()
  message(FATAL_ERROR "The compiler ${CMAKE_C_COMPILER_STEM} is not supported at the moment.")
endif()