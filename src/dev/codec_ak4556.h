#pragma once
#ifndef DSY_CODEC_AK4556_H
#define DSY_CODEC_AK4556_H

#include "daisy_core.h"
#include "per/gpio.h"

namespace daisy
{
/** @brief Ak4556 Codec support 
 ** @author shensley
 ** @addtogroup codec
 ** 
 ** I don't see any real reason to have this be more than a function, but in case we want
 ** to add other functions down the road I wrapped the function in a class.
 ***/
class Ak4556
{
  public:
    Ak4556() {}
    ~Ak4556() {}

    /** Initialization function for Ak4556 
     ** */
    void Init(Pin reset_pin);

    /** Deinitialization function for Ak4556 
     ** */
    void DeInit();

  private:
    GPIO reset_;
};
} // namespace daisy

#endif
