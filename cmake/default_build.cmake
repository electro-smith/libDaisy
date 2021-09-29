add_subdirectory(${LIBDAISY_DIR} libdaisy)
if (${DAISYSP_DIR})
  add_subdirectory(${DAISYSP_DIR} DaisySP)
  set(DAISYSP_LIB DaisySP)
endif()

set(LINKER_SCRIPT ${LIBDAISY_DIR}/core/STM32H750IB_flash.lds)

add_executable(${FIRMWARE_NAME} ${FIRMWARE_SOURCES})

target_link_libraries(${FIRMWARE_NAME}
  PRIVATE
  daisy
  ${DAISYSP_LIB}
  )

set_target_properties(${FIRMWARE_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    SUFFIX ".elf"
    LINK_DEPENDS ${LINKER_SCRIPT}
)

target_link_options(${FIRMWARE_NAME} PUBLIC
    -T ${LINKER_SCRIPT}
    -Wl,-Map=${FIRMWARE_NAME}.map,--cref
    -Wl,--check-sections
    -Wl,--unresolved-symbols=report-all
    -Wl,--warn-common
    -Wl,--warn-section-align
    -Wl,--print-memory-usage
)

add_custom_command(TARGET ${FIRMWARE_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
    ARGS -O ihex
    -S ${FIRMWARE_NAME}.elf
    ${FIRMWARE_NAME}.hex
    BYPRODUCTS
    ${FIRMWARE_NAME}.hex
    COMMENT "Generating HEX image"
VERBATIM)

add_custom_command(TARGET ${FIRMWARE_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
    ARGS -O binary
    -S ${FIRMWARE_NAME}.elf
    ${FIRMWARE_NAME}.bin
    BYPRODUCTS
    ${FIRMWARE_NAME}.bin
    COMMENT "Generating binary image"
VERBATIM)
