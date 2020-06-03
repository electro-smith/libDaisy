#pragma once
#ifndef DSY_CORE_HW_H
#define DSY_CORE_HW_H /**< # */
#include <stdint.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline /**< # */
#elif defined(__clang__)
#define FORCE_INLINE inline __attribute__((always_inline)) /**< # */
#pragma clang diagnostic ignored "-Wduplicate-decl-specifier"
#elif defined(__GNUC__)
#define FORCE_INLINE inline __attribute__((always_inline)) /**< # */
#else
#error unknown compiler
#endif

/** @file daisy_core.h */

/** Macro for area of memory that is configured as cacheless
This should be used primarily for DMA buffers, and the like.
*/
#define DMA_BUFFER_MEM_SECTION __attribute__((section(".sram1_bss")))
/** 
THE DTCM RAM section is also non-cached. However, is not suitable 
for DMA transfers. Performance is on par with internal SRAM w/ 
cache enabled.
*/
#define DTCM_MEM_SECTION __attribute__((section(".dtcmram_bss")))

/** 
    Computes cube.
    \param x Number to be cubed
    \return x ^ 3
*/
FORCE_INLINE float cube(float x)
{
    return (x * x) * x;
}

/** Enums and a simple struct for defining a hardware pin on the MCU
These correlate with the stm32 datasheet, and are used to configure
the hardware.
*/
typedef enum
{
    DSY_GPIOA, /**< # */
    DSY_GPIOB, /**< # */
    DSY_GPIOC, /**< # */
    DSY_GPIOD, /**< # */
    DSY_GPIOE, /**< # */
    DSY_GPIOF, /**< # */
    DSY_GPIOG, /**< # */
    DSY_GPIOH, /**< # */
    DSY_GPIOI, /**< # */
    DSY_GPIOJ, /**< # */
    DSY_GPIOK, /**< # */
    DSY_GPIOX, /** This is a non-existant port for unsupported bits of hardware. */
    DSY_GPIO_LAST, /** Final enum member */
} dsy_gpio_port;

/** Hardware define pins */
typedef struct
{
    dsy_gpio_port port; /**< # */
    uint8_t       pin;  /**< number 0-15 */
} dsy_gpio_pin;

/** Helper for creating pins from port/pin combos easily
*/
FORCE_INLINE dsy_gpio_pin dsy_pin(dsy_gpio_port port, uint8_t pin)
{
    dsy_gpio_pin p;
    p.port = port;
    p.pin  = pin;
    return p;
}

/** Helper for testing sameness of two dsy_gpio_pins
    \return    1 if same, 0 if different
*/
FORCE_INLINE uint8_t dsy_pin_cmp(dsy_gpio_pin *a, dsy_gpio_pin *b)
{
    return ((a->port == b->port) && (a->pin == b->pin));
}

#endif
