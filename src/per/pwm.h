#pragma once
#ifndef DSY_PWM_H
#define DSY_PWM_H

#include "daisy_core.h"
#include "stm32h7xx_hal.h"
#include <cstdint>

namespace daisy
{
/** @brief Hardware PWM using the timer peripheral.
 *
 * * Supports the following TIM peripherals:
 * - TIM3, TIM4, TIM5
 *
 * A single TIM peripheral can be used to control up to four PWM output
 * channels, which share the same resolution and frequency but have independent
 * duty cycles.
 *
 * Note that PWM interferes with the use of a TimerHandle on the same timer.
 *
 * Some channels have the option to choose which output pin they connect to.
 * Only one pin can be connected to a given channel. Use the following table to
 * determine which pin corresponds to each timer and channel.
 *
 * TIM3:
 * - Channel 1: PA6 or PB4 (Daisy Seed -- D19, D9; default PA6/D19)
 * - Channel 2: PA7, PB5, or PC7 (D18, D10, internal LED; default PC7/LED)
 * - Channel 3: PC8 (D4)
 * - Channel 4: PB1 or PC9 (D17, D3; default PB1/D17)
 *
 * TIM4:
 * - Channel 1: PB6 (D13)
 * - Channel 2: PB7 (D14)
 * - Channel 3: PB8 (D11)
 * - Channel 4: PB9 (D12)
 *
 * TIM5:
 * - Channel 1: PA0 (D25)
 * - Channel 2: PA1 (D24)
 * - Channel 3: PA2 (D28)
 * - Channel 4: PA3 (D16)
 *
 * Future work:
 * - Support other timers, including HRTIM, TIM1, TIM8
 * - DMA
 */
class PWMHandle
{
  public:
    class Impl;

    /** @brief Configuration struct for the timer peripheral
     *  @note These settings are used during initialization
     *   and changing them afterwards may not have the desired effect.
     */
    struct Config
    {
        /** @brief Hardware Timer to use for PWM. */
        enum class Peripheral
        {
            TIM_3 = 0, /**< 16-bit counter (max period 0xffff) */
            TIM_4 = 1, /**< 16-bit counter (max period 0xffff) */
            TIM_5 = 2  /**< 32-bit counter (max period 0xffffffff) */
        };
        Peripheral periph; /**< Hardware Peripheral */

        /** @brief Prescaler that divides the PWM timer frequency. The final
         * frequency will be sysclk / (2 * (period + 1) * (prescaler + 1)).
         */
        uint32_t prescaler;

        /** @brief period in ticks at TIM frequency before the counter resets.
         * Affects both the frequency and resolution of PWM.
         *  @note TIM3 and TIM4 are both 16-bit timers so the max period is 0xffff.
         * TIM5 is a 32-bit timer so the max period is 0xffffffff (about 20 seconds
         * per reset).
         */
        uint32_t period;

        Config() : periph(Peripheral::TIM_3), prescaler(0), period(0xffff) {}
        Config(Peripheral periph_,
               uint32_t   prescaler_ = 0,
               uint32_t   period_    = 0xffff)
        : periph(periph_), prescaler(prescaler_), period(period_)
        {
        }
    };

    /** @brief Return values for PWM functions. */
    enum class Result
    {
        OK  = 0,
        ERR = 1,
    };

    class Channel
    {
      public:
        /** @brief Configuration struct for an individual channel
         *  @note These settings are used during initialization
         *   and changing them afterwards may not have the desired effect.
         */
        struct Config
        {
            /** @brief Pin to use for this channel. Ensure that this is the proper pin
             * for the timer and channel. Use PORTX (default) to select the channel's
             * default pin.
             */
            Pin pin;

            /** @brief Output polarity */
            enum class Polarity
            {
                HIGH = 0, /**< Output is high when channel is active */
                LOW       /**< Output is low when channel is active */
            };
            Polarity polarity;

            Config() : pin(), polarity(Polarity::HIGH) {}
            Config(Pin pin_, Polarity polarity_ = Polarity::HIGH)
            : pin(pin_), polarity(polarity_)
            {
            }
        };

        /** @brief Private constructor for channel. Do not use. */
        Channel(PWMHandle *owner, uint32_t channel)
        : owner_(*owner), channel_(channel), scale_(65535.0f), handle_(nullptr)
        {
        }

        /** @brief Returns a const reference to the Config struct */
        inline const Config &GetConfig() const { return config_; }

        /** @brief Initialize the channel. Must be called manually, after
         * PWMHandle::Init  */
        PWMHandle::Result Init(const Channel::Config &config);

        /** @brief Initialize the channel using all defaults. Must be called
         * manually, after PWMHandle::Init  */
        PWMHandle::Result Init();

        /** @brief Deinitialize the channel. Called automatically by
         * PWMHandle::DeInit */
        PWMHandle::Result DeInit();

        /** @brief Set the duty cycle for the PWM channel.
         * \param raw Must be less than or equal to the timer's period
         */
        inline void SetRaw(uint32_t raw)
        {
            __HAL_TIM_SET_COMPARE(handle_, channel_, raw);
        }

        /** @brief Set the duty cycle for the PWM channel. Automatically
         * normalized to the timer's period.
         * \param val Relative value, [0.0, 1.0]
         * \note May experience rounding errors when period is > 2^24; use SetRaw.
         */
        inline void Set(float val)
        {
            if(val < 0.0f)
                val = 0.0f;
            if(val > 1.0f)
                val = 1.0f;
            SetRaw(static_cast<uint32_t>(val * scale_));
        }

      private:
        PWMHandle &        owner_;
        const uint32_t     channel_;
        Channel::Config    config_;
        float              scale_;
        TIM_HandleTypeDef *handle_;
    };

    PWMHandle();

    PWMHandle(const PWMHandle &other) = default;
    PWMHandle &operator=(const PWMHandle &other) = default;
    ~PWMHandle() {}

    /** @brief Initialize the PWM peripheral according to the config */
    Result Init(const Config &config);

    /** @brief Deinitialize the peripheral */
    Result DeInit();

    /** @brief Returns a const reference to the Config struct */
    const Config &GetConfig() const;

    /** @brief Get a reference to CH1 of this peripheral. Must be initialized
     * before use. */
    inline Channel &Channel1() { return ch1_; }

    /** @brief Get a reference to CH2 of this peripheral. Must be initialized
     * before use. */
    inline Channel &Channel2() { return ch2_; }

    /** @brief Get a reference to CH3 of this peripheral. Must be initialized
     * before use. */
    inline Channel &Channel3() { return ch3_; }

    /** @brief Get a reference to CH4 of this peripheral. Must be initialized
     * before use. */
    inline Channel &Channel4() { return ch4_; };

    /** @brief Set the prescaler */
    void SetPrescaler(uint32_t prescaler);

    /** @brief Set the period */
    void SetPeriod(uint32_t period);

  private:
    Impl *pimpl_;

    // NOTE: These are stored here, not in the Impl class, so that channel
    // references are valid even if taken before a call to PWMHandle::Init
    Channel ch1_;
    Channel ch2_;
    Channel ch3_;
    Channel ch4_;
};

} // namespace daisy

#endif
