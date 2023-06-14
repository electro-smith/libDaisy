find_package(Threads REQUIRED)
add_subdirectory(googletest)

add_executable(daisy_test
  AbstractMenu_gtest.cpp
  CpuLoadMeter_gtest.cpp
  FIFO_gtest.cpp
  FixedCapStr_gtest.cpp
  libDaisyCombined.cpp
  MappedValue_gtest.cpp
  MAX11300_gtest.cpp
  Midi_gtest.cpp
  PersistentStorage_gtest.cpp
  Pin_gtest.cpp
  Rectangle_gtest.cpp
  Stack_gtest.cpp
  VoctCalibration_gtest.cpp
)

target_link_libraries(daisy_test
  daisy
  Threads::Threads
  GTest::gtest_main
)

target_compile_definitions(daisy_test PRIVATE
  UNIT_TEST
)

set_target_properties(daisy_test PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED
  C_STANDARD 11
  C_STANDARD_REQUIRED
)

include(GoogleTest)
gtest_discover_tests(daisy_test)
