#pragma once
#ifndef DSY_CORE_HW_H
#define DSY_CORE_HW_H /**< & */
#include <stdint.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline /**< & */
#elif defined(__clang__)
#define FORCE_INLINE inline __attribute__((always_inline)) /**< & */
#pragma clang diagnostic ignored "-Wduplicate-decl-specifier"
#elif defined(__GNUC__)
#define FORCE_INLINE inline __attribute__((always_inline)) /**< & */
#else
#error unknown compiler
#endif

/** @addtogroup utility
    @{
*/

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

#define FBIPMAX 0.999985f             /**< close to 1.0f-LSB at 16 bit */
#define FBIPMIN (-FBIPMAX)            /**< - (1 - LSB) */
#define U82F_SCALE 0.0078740f         /**< 1 / 127 */
#define F2U8_SCALE 127.0f             /**< 128 - 1 */
#define S82F_SCALE 0.0078125f         /**< 1 / (2**7) */
#define F2S8_SCALE 127.0f             /**< (2 ** 7) - 1 */
#define S162F_SCALE 3.0517578125e-05f /**< 1 / (2** 15) */
#define F2S16_SCALE 32767.0f          /**< (2 ** 15) - 1 */
#define F2S24_SCALE 8388608.0f        /**< 2 ** 23 */
#define S242F_SCALE 1.192092896e-07f  /**< 1 / (2 ** 23) */
#define S24SIGN 0x800000              /**< 2 ** 23 */
#define S322F_SCALE 4.6566129e-10f    /**< 1 / (2** 31) */
#define F2S32_SCALE 2147483647.f      /**< (2 ** 31) - 1 */


/** shorthand macro for simplifying the reading of the left 
 *  channel of a non-interleaved output buffer named out */
#define OUT_L out[0]

/** shorthand macro for simplifying the reading of the right 
 *  channel of a non-interleaved output buffer named out */
#define OUT_R out[1]

/** shorthand macro for simplifying the reading of the left 
 *  channel of a non-interleaved input buffer named in */
#define IN_L in[0]

/** shorthand macro for simplifying the reading of the right 
 *  channel of a non-interleaved input buffer named in */
#define IN_R in[1]

/** 
    Computes cube.
    \param x Number to be cubed
    \return x ^ 3
*/
FORCE_INLINE float cube(float x)
{
    return (x * x) * x;
}

/** 
    Converts unsigned 8-bit to float
    \param x Number to be scaled.
    \return Scaled number.
*/
FORCE_INLINE float u82f(uint8_t x)
{
    return ((float)x - 127.f) * U82F_SCALE;
}

/**
    Converts float to unsigned 8-bit
*/
FORCE_INLINE uint8_t f2u8(float x)
{
    x = x <= FBIPMIN ? FBIPMIN : x;
    x = x >= FBIPMAX ? FBIPMAX : x;
    return (uint8_t)((x * F2U8_SCALE) + F2U8_SCALE);
}


/** 
    Converts Signed 8-bit to float
    \param x Number to be scaled.
    \return Scaled number.
*/
FORCE_INLINE float s82f(int8_t x)
{
    return (float)x * S82F_SCALE;
}

/**
    Converts float to Signed 8-bit
*/
FORCE_INLINE int8_t f2s8(float x)
{
    x = x <= FBIPMIN ? FBIPMIN : x;
    x = x >= FBIPMAX ? FBIPMAX : x;
    return (int32_t)(x * F2S8_SCALE);
}

/** 
    Converts Signed 16-bit to float
    \param x Number to be scaled.
    \return Scaled number.
*/
FORCE_INLINE float s162f(int16_t x)
{
    return (float)x * S162F_SCALE;
}

/**
    Converts float to Signed 16-bit
*/
FORCE_INLINE int16_t f2s16(float x)
{
    x = x <= FBIPMIN ? FBIPMIN : x;
    x = x >= FBIPMAX ? FBIPMAX : x;
    return (int32_t)(x * F2S16_SCALE);
}

/**
    Converts Signed 24-bit to float
 */
FORCE_INLINE float s242f(int32_t x)
{
    x = (x ^ S24SIGN) - S24SIGN; //sign extend aka ((x<<8)>>8)
    return (float)x * S242F_SCALE;
}
/**
    Converts float to Signed 24-bit
 */
FORCE_INLINE int32_t f2s24(float x)
{
    x = x <= FBIPMIN ? FBIPMIN : x;
    x = x >= FBIPMAX ? FBIPMAX : x;
    return (int32_t)(x * F2S24_SCALE);
}

/**
    Converts Signed 32-bit to float
 */
FORCE_INLINE float s322f(int32_t x)
{
    return (float)x * S322F_SCALE;
}
/**
    Converts float to Signed 24-bit
 */
FORCE_INLINE int32_t f2s32(float x)
{
    x = x <= FBIPMIN ? FBIPMIN : x;
    x = x >= FBIPMAX ? FBIPMAX : x;
    return (int32_t)(x * F2S32_SCALE);
}


/** Enums and a simple struct for defining a hardware pin on the MCU
 *  These correlate with the stm32 datasheet, and are used to configure
 *  the hardware.
 * 
 *  This along with the dsy_gpio_pin class should no longer be used.
 *  They are available for backwards compatability. 
 * 
 *  Please use GPIOPort enum and the Pin struct instead.
 */
typedef enum
{
    DSY_GPIOA, /**< & */
    DSY_GPIOB, /**< & */
    DSY_GPIOC, /**< & */
    DSY_GPIOD, /**< & */
    DSY_GPIOE, /**< & */
    DSY_GPIOF, /**< & */
    DSY_GPIOG, /**< & */
    DSY_GPIOH, /**< & */
    DSY_GPIOI, /**< & */
    DSY_GPIOJ, /**< & */
    DSY_GPIOK, /**< & */
    DSY_GPIOX, /** This is a non-existant port for unsupported bits of hardware. */
    DSY_GPIO_LAST, /** Final enum member */
} dsy_gpio_port;

/** Hardware define pins 
 *  
 *  The dsy_gpio_pin struct should no longer be used, and is only available for
 *  backwards compatability.
 * 
 *  Please use Pin struct instead.
 */
typedef struct
{
    dsy_gpio_port port; /**< & */
    uint8_t       pin;  /**< number 0-15 */
} dsy_gpio_pin;

/** Helper for creating pins from port/pin combos easily
 * 
 *  The dsy_gpio_pin struct should no longer be used, and is only available for
 *  backwards compatability.
 * 
 *  Please use Pin struct instead.
*/
FORCE_INLINE dsy_gpio_pin dsy_pin(dsy_gpio_port port, uint8_t pin)
{
    dsy_gpio_pin p;
    p.port = port;
    p.pin  = pin;
    return p;
}

/** Helper for testing sameness of two dsy_gpio_pins
 *  \return    1 if same, 0 if different
 * 
 *  The dsy_gpio_pin struct should no longer be used, and is only available for
 *  backwards compatability.
 * 
 *  Please use Pin struct instead.
 */
FORCE_INLINE uint8_t dsy_pin_cmp(dsy_gpio_pin *a, dsy_gpio_pin *b)
{
    return ((a->port == b->port) && (a->pin == b->pin));
}

#ifdef __cplusplus

namespace daisy
{
/** @brief GPIO Port names */
enum GPIOPort
{
    PORTA, /**< Port A */
    PORTB, /**< Port B */
    PORTC, /**< Port C */
    PORTD, /**< Port D */
    PORTE, /**< Port E */
    PORTF, /**< Port F */
    PORTG, /**< Port G */
    PORTH, /**< Port H */
    PORTI, /**< Port I */
    PORTJ, /**< Port J */
    PORTK, /**< Port K */
    PORTX, /**< Used as a dummy port to signal an invalid pin. */
};

/** @brief representation of hardware port/pin combination */
struct Pin
{
    GPIOPort port;
    uint8_t  pin;

    /** @brief Constructor creates a valid pin. 
     *  @param pt GPIOPort between PA, and PK corresponding to STM32 Port.
     *  @param pn pin number in range of 0-15
    */
    constexpr Pin(const GPIOPort pt, const uint8_t pn) : port(pt), pin(pn) {}

    /** @brief Basic Constructor creates an invalid Pin object */
    constexpr Pin() : port(PORTX), pin(255) {}

    /** @brief checks validity of a Pin 
     *  @retval returns true if the port is a valid hardware pin, otherwise false.
    */
    constexpr bool IsValid() const { return port != PORTX && pin < 16; }

    /** @brief comparison operator for checking equality between Pin objects */
    constexpr bool operator==(const Pin &rhs) const
    {
        return (rhs.port == port) && (rhs.pin == pin);
    }

    /** @brief comparison operator for checking inequality between Pin objects */
    constexpr bool operator!=(const Pin &rhs) const { return !operator==(rhs); }

    /** @brief conversion operation for converting to the old-style representation
     *         of a pin.
     * 
     *  This allows the new Pin type to be used in place of the older, dsy_gpio_pin
     *  type.
     */
    constexpr operator dsy_gpio_pin() const
    {
        return dsy_gpio_pin{.port = static_cast<dsy_gpio_port>(port),
                            .pin  = pin};
    }
};

} // namespace daisy

#endif // __cplusplus

#endif
/** @} */
