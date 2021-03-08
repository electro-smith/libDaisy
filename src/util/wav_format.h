#pragma once
#ifndef DSY_WAV_FORMAT_H
#define DSY_WAV_FORMAT_H

#include <stdint.h>

/** @addtogroup utility
    @{
*/


namespace daisy
{
/** Constants for In-Header IDs */
const uint32_t kWavFileChunkId     = 0x46464952; /**< "RIFF" */
const uint32_t kWavFileWaveId      = 0x45564157; /**< "WAVE" */
const uint32_t kWavFileSubChunk1Id = 0x20746d66; /**< "fmt " */
const uint32_t kWavFileSubChunk2Id = 0x61746164; /**< "data" */

/** Standard Format codes for the waveform data.
 ** 
 ** According to spec, extensible should be used whenever:
 ** * PCM data has more than 16 bits/sample
 ** * The number of channels is more than 2
 ** * The actual number of bits/sample is not equal to the container size
 ** * The mapping from channels to speakers needs to be specified.
 ** */
enum WavFileFormatCode
{
    WAVE_FORMAT_PCM        = 0x0001,
    WAVE_FORMAT_IEEE_FLOAT = 0x0003,
    WAVE_FORMAT_ALAW       = 0x0006,
    WAVE_FORMAT_ULAW       = 0x0007,
    WAVE_FORMAT_EXTENSIBLE = 0xFFFE,
};

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

} // namespace daisy

#endif
/** @} */
