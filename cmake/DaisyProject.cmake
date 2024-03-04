set(DAISY_STORAGE flash CACHE STRING "Select the storage destination of the executable.")

if(DEFINED CUSTOM_LINKER_SCRIPT)
    set(LINKER_SCRIPT ${CUSTOM_LINKER_SCRIPT})
else()
    cmake_path(SET LINKER_SCRIPT NORMALIZE ${CMAKE_CURRENT_LIST_DIR}/../core/STM32H750IB_${DAISY_STORAGE}.lds)
endif()

# Global optimization options
if(NOT DEFINED FIRMWARE_DEBUG_OPT_LEVEL)
    # Enables optimizations that do not interfere with debugging.
    add_compile_options($<$<CONFIG:Debug>:-Og>)
else()
    add_compile_options($<$<CONFIG:Debug>:${FIRMWARE_DEBUG_OPT_LEVEL}>)
endif()

if(NOT DEFINED FIRMWARE_RELEASE_OPT_LEVEL)
    add_compile_options($<$<CONFIG:Release>:-O3>)
else()
    add_compile_options($<$<CONFIG:Release>:${FIRMWARE_RELEASE_OPT_LEVEL}>)
endif()

if(NOT DEFINED FIRMWARE_MINSIZEREL_OPT_LEVEL)
    # Optimize for size. -Os enables all -O2 optimizations.
    add_compile_options($<$<CONFIG:MinSizeRel>:-Os>)
else()
    add_compile_options($<$<CONFIG:MinSizeRel>:${FIRMWARE_MINSIZEREL_OPT_LEVEL}>)
endif()

# Always use LTO on Release
add_compile_options($<$<CONFIG:Release,MinSizeRel,RelWithDebInfo>:-flto>)
add_link_options($<$<CONFIG:Release,MinSizeRel,RelWithDebInfo>:-flto>)

# Include debug symbols when compiling with debug flags
add_compile_options($<$<CONFIG:Debug,RelWithDebInfo>:-ggdb3>)

# Define NDEBUG when not compiling with debug flags
add_compile_definitions($<$<CONFIG:Release,MinSizeRel>:NDEBUG>)

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
    COMMAND ${CMAKE_OBJCOPY} -O ihex -S $<TARGET_FILE:${FIRMWARE_NAME}> $<TARGET_FILE_DIR:${FIRMWARE_NAME}>/${FIRMWARE_NAME}.hex
    BYPRODUCTS ${FIRMWARE_NAME}.hex
    DEPENDS ${FIRMWARE_NAME}
    COMMENT "Generating HEX image"
    VERBATIM
)

option(DAISY_GENERATE_BIN "Sets whether or not to generate a raw binary image using objcopy (warning this is a very large file, as it is a representation of the *full memory space*.)")

if(DAISY_GENERATE_BIN)
    add_custom_command(TARGET ${FIRMWARE_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary -S $<TARGET_FILE:${FIRMWARE_NAME}> $<TARGET_FILE_DIR:${FIRMWARE_NAME}>/${FIRMWARE_NAME}.bin
        BYPRODUCTS ${FIRMWARE_NAME}.bin
        DEPENDS ${FIRMWARE_NAME}
        COMMENT "Generating binary image"
        VERBATIM
    )
endif()
