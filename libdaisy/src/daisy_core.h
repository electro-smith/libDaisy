// # Core
#pragma once
#ifndef DSY_CORE_HW_H
#define DSY_CORE_HW_H
#include <stdint.h>
#include <stdlib.h>

// ## Memory Section Macros
//
// Macro for area of memory that is configured as cacheless
// This should be used primarily for DMA buffers, and the like.
#define DMA_BUFFER_MEM_SECTION __attribute__((section(".sram1_bss")))
// 
// THE DTCM RAM section is also non-cached. However, is not suitable 
// for DMA transfers. Performance is on par with internal SRAM w/ 
// cache enabled.
#define DTCM_MEM_SECTION __attribute__((section(".dtcmram_bss")))

// ## GPIO
// Enums and a simple struct for defining a hardware pin on the MCU
// These correlate with the stm32 datasheet, and are used to configure
// the hardware.
typedef enum
{
    DSY_GPIOA,
    DSY_GPIOB,
    DSY_GPIOC,
    DSY_GPIOD,
    DSY_GPIOE,
    DSY_GPIOF,
    DSY_GPIOG,
    DSY_GPIOH,
    DSY_GPIOI,
    DSY_GPIOJ,
    DSY_GPIOK,
    DSY_GPIOX, // This is a non-existant port for unsupported bits of hardware.
    DSY_GPIO_LAST,
} dsy_gpio_port;

typedef struct
{
    dsy_gpio_port port;
	uint8_t pin; // number 0-15
} dsy_gpio_pin;


#endif
