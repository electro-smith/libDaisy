# Driver Notes

This can be used to keep track of any manual changes to the HAL or CMSIS files to make migrating to new versions easier in the future.

* In CMSIS/Devices/ST/STM32H7xx/Include/stm32h7xx.h
    * uncommented line 66 for `STM32H750xx`, though this is also passed in with `-D` in the cflags of builds.
* In Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_spi.c added and `ifndef` around `assert_param` to prevent redeclration warning
* In Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c, `__HAL_RCC_AHB3_FORCE_RESET()` and its release (in `HAL_DeInit`) are commented out to avoid a system reset in the bootloader
