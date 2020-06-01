/** Driver for the AK4556 Stereo Codec
*/
#pragma once
#ifndef DSY_CODEC_AK4556_H
#define DSY_CODEC_AK4556_H

#include "daisy_core.h"

/** Resets the AK4556
*/
//
/** reset_pin should be a dsy_gpio_pin that is connected to the RST pin of the AK4556
*/
void codec_ak4556_init(dsy_gpio_pin reset_pin);

#endif
