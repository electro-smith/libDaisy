find_package(Threads REQUIRED)
add_subdirectory(googletest)
include(GoogleTest)

# Add a gtest
function(add_gtest source_file)
  cmake_path(GET source_file STEM test_name)

  # Create the target
  add_executable(${test_name} ${source_file})

  # Link it against the required libraries
  target_link_libraries(${test_name} PUBLIC
    daisy
    Threads::Threads
    GTest::gtest_main
  )

  # Set any compile definitions
  target_compile_definitions(${test_name} PRIVATE
    UNIT_TEST
  )

  # Set the properties
  set_target_properties(${test_name} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
  )

  # Add the target as a test
  add_test(NAME ${test_name} COMMAND ${test_name} --verbose)

  # Discover unit tests inside it
  gtest_discover_tests(${test_name})
endfunction(add_gtest)

# Generate test executables from files ending in *_gtest.cpp
function(autogen_gtests test_folder)
  file(GLOB_RECURSE specs ${test_folder}/*_gtest.cpp)

  foreach(spec IN LISTS specs)
    add_gtest(${spec})
  endforeach()
endfunction()

autogen_gtests(${CMAKE_CURRENT_LIST_DIR} daisy)
