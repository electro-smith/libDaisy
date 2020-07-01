#pragma once
#ifndef DSY_WAV_FORMAT_H
#define DSY_WAV_FORMAT_H

#include <stdint.h>

/** @addtogroup utility
    @{
*/

/** Helper struct for handling the WAV file format */

typedef struct
{
    uint32_t ChunkId;       /**< & */
    uint32_t FileSize;      /**< & */
    uint32_t FileFormat;    /**< & */
    uint32_t SubChunk1ID;   /**< & */
    uint32_t SubChunk1Size; /**< & */
    uint16_t AudioFormat;   /**< & */
    uint16_t NbrChannels;   /**< & */
    uint32_t SampleRate;    /**< & */
    uint32_t ByteRate;      /**< & */
    uint16_t BlockAlign;    /**< & */
    uint16_t BitPerSample;  /**< & */
    uint32_t SubChunk2ID;   /**< & */
    uint32_t SubCHunk2Size; /**< & */
} WAV_FormatTypeDef;

#endif
/** @} */
