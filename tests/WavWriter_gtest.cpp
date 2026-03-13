#include <gtest/gtest.h>
#include <cstring>

#include "daisy_core.h"
#include "FatFsMock.h"
#include "util/WavWriter.h"

using namespace daisy;

static WAV_FormatTypeDef ReadHeaderFromMockFile()
{
    const auto& state = test::GetFatFsMockState();
    WAV_FormatTypeDef header;
    std::memcpy(&header, state.file_data.data(), sizeof(header));
    return header;
}

TEST(util_WavWriter, saveFileUpdatesHeaderSizesCorrectly)
{
    test::ResetFatFsMockState();

    WavWriter<32> writer;
    WavWriter<32>::Config cfg;
    cfg.samplerate    = 48000.0f;
    cfg.channels      = 1;
    cfg.bitspersample = 16;

    writer.Init(cfg);
    writer.OpenFile("header_size_test.wav");
    ASSERT_TRUE(writer.IsRecording());

    float frame = 0.25f;
    for(size_t i = 0; i < 5; i++)
        writer.Sample(&frame);

    writer.SaveFile();

    const auto&       state       = test::GetFatFsMockState();
    const auto        header      = ReadHeaderFromMockFile();
    const size_t      payload_len = state.file_data.size() - sizeof(WAV_FormatTypeDef);
    const uint32_t    subchunk2   = header.SubCHunk2Size;
    const uint32_t    filesize    = header.FileSize;

    EXPECT_EQ(subchunk2, payload_len);
    EXPECT_EQ(filesize, 36u + subchunk2);
}

TEST(util_WavWriter, openAndSaveWithZeroSamplesProducesValidEmptyWav)
{
    test::ResetFatFsMockState();

    WavWriter<32> writer;
    WavWriter<32>::Config cfg;
    cfg.samplerate    = 44100.0f;
    cfg.channels      = 2;
    cfg.bitspersample = 16;

    writer.Init(cfg);
    writer.OpenFile("empty.wav");
    ASSERT_TRUE(writer.IsRecording());
    EXPECT_EQ(writer.GetLengthSamps(), 0u);

    writer.SaveFile();

    const auto& state = test::GetFatFsMockState();
    ASSERT_EQ(state.file_data.size(), sizeof(WAV_FormatTypeDef));

    const auto header = ReadHeaderFromMockFile();
    EXPECT_EQ(header.ChunkId, kWavFileChunkId);
    EXPECT_EQ(header.FileFormat, kWavFileWaveId);
    EXPECT_EQ(header.SubChunk2ID, kWavFileSubChunk2Id);
    EXPECT_EQ(header.SubCHunk2Size, 0u);
    EXPECT_EQ(header.FileSize, 36u);
    EXPECT_EQ(writer.GetLengthSamps(), 0u);
}

TEST(util_WavWriter, lengthAccessorsRemainConsistentWithSavedPayload)
{
    test::ResetFatFsMockState();

    WavWriter<64> writer;
    WavWriter<64>::Config cfg;
    cfg.samplerate    = 32000.0f;
    cfg.channels      = 2;
    cfg.bitspersample = 32;

    writer.Init(cfg);
    writer.OpenFile("length_consistency.wav");
    ASSERT_TRUE(writer.IsRecording());
    EXPECT_EQ(writer.GetLengthSamps(), 0u);

    float  frame[2]     = {0.1f, -0.1f};
    size_t num_frames   = 7;
    for(size_t i = 0; i < num_frames; i++)
        writer.Sample(frame);

    const uint32_t length_before_save = writer.GetLengthSamps();
    EXPECT_EQ(length_before_save, num_frames);
    EXPECT_FLOAT_EQ(writer.GetLengthSeconds(), num_frames / cfg.samplerate);

    writer.SaveFile();

    const auto& state = test::GetFatFsMockState();
    const auto  header = ReadHeaderFromMockFile();

    const uint32_t expected_payload_bytes
        = length_before_save * cfg.channels * (cfg.bitspersample / 8);
    const size_t actual_payload_bytes = state.file_data.size() - sizeof(WAV_FormatTypeDef);

    EXPECT_EQ(header.SubCHunk2Size, expected_payload_bytes);
    EXPECT_EQ(actual_payload_bytes, expected_payload_bytes);
    EXPECT_EQ(header.FileSize, 36u + expected_payload_bytes);
    EXPECT_EQ(writer.GetLengthSamps(), 0u);
}
