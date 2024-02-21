set(DAISY_STORAGE flash CACHE STRING "Select the storage destination of the executable.")

if(DEFINED CUSTOM_LINKER_SCRIPT)
    set(LINKER_SCRIPT ${CUSTOM_LINKER_SCRIPT})
else()
    cmake_path(SET LINKER_SCRIPT NORMALIZE ${CMAKE_CURRENT_LIST_DIR}/../core/STM32H750IB_${DAISY_STORAGE}.lds)
endif()

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

target_compile_options(${FIRMWARE_NAME} PUBLIC
    $<$<CONFIG:RELEASE>:-flto>
)

target_link_options(${FIRMWARE_NAME} PUBLIC
    LINKER:-T,${LINKER_SCRIPT}
    $<$<CONFIG:DEBUG>:LINKER:-Map=${FIRMWARE_NAME}.map>
    $<$<CONFIG:DEBUG>:LINKER:--cref>
    LINKER:--gc-sections
    LINKER:--check-sections
    LINKER:--unresolved-symbols=report-all
    LINKER:--warn-common
    $<$<CXX_COMPILER_ID:GNU>:LINKER:--warn-section-align>

    # Currently a GSoC project to port this to LLD
    $<$<CXX_COMPILER_ID:GNU>:LINKER:--print-memory-usage>
)

add_custom_command(TARGET ${FIRMWARE_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex -S ${FIRMWARE_NAME}.elf ${FIRMWARE_NAME}.hex
    BYPRODUCTS ${FIRMWARE_NAME}.hex
    COMMENT "Generating HEX image"
    VERBATIM
)

option(DAISY_GENERATE_BIN "Sets whether or not to generate a raw binary image using objcopy (warning this is a very large file, as it is a representation of the *full memory space*.)")

if(DAISY_GENERATE_BIN)
    add_custom_command(TARGET ${FIRMWARE_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary -S ${FIRMWARE_NAME}.elf ${FIRMWARE_NAME}.bin
        BYPRODUCTS ${FIRMWARE_NAME}.bin
        COMMENT "Generating binary image"
        VERBATIM
    )
endif(DAISY_GENERATE_BIN)