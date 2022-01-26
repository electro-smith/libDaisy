#include "dev/hall_sensor.h"

namespace daisy
{
HallSensor::Result HallSensor::Init(HallSensor::Config config)
{
    config_     = config;
    count_      = 0;
    last_state_ = 0;

    Switch::Polarity pol;
    switch(config_.polarity)
    {
        case NORMAL: pol = Switch::Polarity::POLARITY_NORMAL; break;
        case INVERTED: pol = Switch::Polarity::POLARITY_INVERTED; break;
    }

    Switch::Pull pull;
    switch(config_.pull)
    {
        case UP: pull = Switch::Pull::PULL_UP; break;
        case DOWN: pull = Switch::Pull::PULL_DOWN; break;
        case NONE: pull = Switch::Pull::PULL_NONE; break;
    }

    sw_.Init(config_.pin, 0.f, Switch::Type::TYPE_MOMENTARY, pol, pull);

    return OK;
}

HallSensor::Result HallSensor::Process()
{
    sw_.Debounce();

    if(config_.edge == RISING || config_.edge == BOTH)
    {
        count_ += sw_.RisingEdge();
    }

    if(config_.edge == FALLING || config_.edge == BOTH)
    {
        count_ += sw_.FallingEdge();
    }

    return OK;
}

} // namespace daisy
