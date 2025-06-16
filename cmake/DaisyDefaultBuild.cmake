add_subdirectory(${LIBDAISY_DIR} libdaisy)
if (DEFINED DAISYSP_DIR)
  add_subdirectory(${DAISYSP_DIR} DaisySP)
  set(DAISYSP_LIB DaisySP)
endif()

include(DaisyProject)