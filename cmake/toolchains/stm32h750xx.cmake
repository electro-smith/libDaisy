set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Architucture flags
set(ARCH_COMBO armv7m_hard_fpv5_d16)
set(MCU
  -mcpu=cortex-m7
  -mfpu=fpv5-d16
  -mfloat-abi=hard
  -mthumb
)
add_compile_options(${MCU})
add_link_options(${MCU})

add_compile_options(
  -fno-exceptions
  -ffunction-sections
  -fdata-sections
  -fomit-frame-pointer
  -finline-functions
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

add_compile_definitions(
  CORE_CM7
  STM32H750xx
  STM32H750IB
  ARM_MATH_CM7
  HSE_VALUE=16000000
)
