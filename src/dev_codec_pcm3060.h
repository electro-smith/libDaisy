#ifndef CODEC_PCM3060_H
#define CODEC_PCM3060_H
#include "per_i2c.h"


/** 
    Driver for the PCM3060 Codec.
    \file dev_codec_pcm3060.h
*/

/** 
    Resets the PCM060
    \param *hi2c array of pins handling i2c?
*/
void codec_pcm3060_init(dsy_i2c_handle *hi2c);
#endif
