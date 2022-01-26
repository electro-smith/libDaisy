#pragma once
#ifndef DSY_HALLSENSOR_H
#define DSY_HALLSENSOR_H

#include "hid/switch.h"

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/** @brief Device support for US5881 Hall Effect Sensor
    @author beserge
    @date December 2021
*/
class HallSensor
{
  public:
    HallSensor() {}
    ~HallSensor() {}

    enum Edge
    {
        RISING,
        FALLING,
        BOTH
    };

    enum Polarity
    {
        NORMAL,
        INVERTED
    };

    enum Pull
    {
        UP,
        DOWN,
        NONE
    };

    struct Config
    {
        Pin      pin;
        Edge     edge;
        Pull     pull;
        Polarity polarity;

        Config()
        {
            pin  = Pin(PORTA, 2);
            pull = NONE;

            edge     = RISING;
            polarity = INVERTED;
        }
    };

    enum Result
    {
        OK = 0,
        ERR
    };

    /** Initialize the Hall Sensor device
        \param config Configuration settings
    */
    Result Init(Config config);

    /** Debounce the hall sensor and update the count */
    Result Process();

    /** Get the total count of hall sensor clicks so far 
        \return Total number of times hall sensor has gone high since init.
    */
    uint32_t GetCount() { return count_; }

    /** Set the counter back to 0 */
    void ResetCount() { count_ = 0; }

    /*** Wrappers ***/

    /** \return true if a magnet just came in range. */
    inline bool RisingEdge() { return sw_.RisingEdge(); }

    /** \return true if a magnet was just left range */
    inline bool FallingEdge() { return sw_.FallingEdge(); }

    /** \return true if a magnet is currently in range */
    inline bool State() { return sw_.Pressed(); }

    /** \return true if a magnet is in range, no debouncing */
    inline bool RawState() { return sw_.RawState(); }

    /** \return Time a magnet has been in range in ms */
    inline float TimeOnMs() { return sw_.TimeHeldMs(); }


  private:
    Config config_;
    Switch sw_;

    bool     last_state_;
    uint32_t count_;

    /** @} */
}; // HallSensor

} // namespace daisy
#endif
