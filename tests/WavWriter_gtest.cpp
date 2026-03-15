#include <gtest/gtest.h>
#include <array>
#include <cstring>
#include <vector>

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

static void SampleMono(WavWriter<16>& writer, const std::vector<float>& samples)
{
    float in[1];
    for(const auto sample : samples)
    {
        in[0] = sample;
        writer.Sample(in);
    }
}

static std::vector<int32_t> ToS32(const std::vector<float>& samples)
{
    std::vector<int32_t> out;
    out.reserve(samples.size());
    for(const auto sample : samples)
        out.push_back(f2s32(sample));
    return out;
}

static std::vector<int32_t> ReadPcmPayloadAsS32Mono()
{
    const auto& state = test::GetFatFsMockState();
    if(state.file_data.size() <= sizeof(WAV_FormatTypeDef))
        return {};

    const size_t payload_size = state.file_data.size() - sizeof(WAV_FormatTypeDef);
    std::vector<int32_t> out(payload_size / sizeof(int32_t));
    std::memcpy(out.data(),
                state.file_data.data() + sizeof(WAV_FormatTypeDef),
                out.size() * sizeof(int32_t));
    return out;
}

template <typename T>
static void AppendBytes(std::vector<uint8_t>& out, T value)
{
    const auto* bytes = reinterpret_cast<const uint8_t*>(&value);
    out.insert(out.end(), bytes, bytes + sizeof(T));
}

static std::vector<uint8_t> ReadPcmPayloadBytes()
{
    const auto& state = test::GetFatFsMockState();
    if(state.file_data.size() <= sizeof(WAV_FormatTypeDef))
        return {};

    return {state.file_data.begin() + sizeof(WAV_FormatTypeDef), state.file_data.end()};
}

static std::vector<uint8_t> ToPcmBytesS32Stereo(
    const std::vector<std::array<float, 2>>& frames)
{
    std::vector<uint8_t> out;
    out.reserve(frames.size() * 2 * sizeof(int32_t));
    for(const auto& frame : frames)
    {
        AppendBytes(out, f2s32(frame[0]));
        AppendBytes(out, f2s32(frame[1]));
    }
    return out;
}

static std::vector<uint8_t> ToPcmBytesS16Mono(const std::vector<float>& samples)
{
    std::vector<uint8_t> out;
    out.reserve(samples.size() * sizeof(int16_t));
    for(const auto sample : samples)
        AppendBytes(out, f2s16(sample));
    return out;
}

static std::vector<uint8_t> ToPcmBytesS16Stereo(
    const std::vector<std::array<float, 2>>& frames)
{
    std::vector<uint8_t> out;
    out.reserve(frames.size() * 2 * sizeof(int16_t));
    for(const auto& frame : frames)
    {
        AppendBytes(out, f2s16(frame[0]));
        AppendBytes(out, f2s16(frame[1]));
    }
    return out;
}

static void SampleStereo(WavWriter<16>&                          writer,
                         const std::vector<std::array<float, 2>>& frames)
{
    float in[2];
    for(const auto& frame : frames)
    {
        in[0] = frame[0];
        in[1] = frame[1];
        writer.Sample(in);
    }
}


TEST(util_WavWriter, saveFile_headerSizesMatchPayload)
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

TEST(util_WavWriter, saveFile_zeroSamplesWritesEmptyWav)
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

TEST(util_WavWriter, saveFile_lengthAccessorsMatchSavedPayload)
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

TEST(util_WavWriter, saveFile_partialFirstHalfTailOnce)
{
    test::ResetFatFsMockState();

    WavWriter<16>         writer;
    WavWriter<16>::Config cfg;
    cfg.samplerate    = 48000.0f;
    cfg.channels      = 1;
    cfg.bitspersample = 32;
    writer.Init(cfg);
    writer.OpenFile("partial_tail.wav");

    const std::vector<float> samples = {0.11f, -0.22f, 0.33f};
    SampleMono(writer, samples);
    writer.SaveFile();

    EXPECT_EQ(ReadPcmPayloadAsS32Mono(), ToS32(samples));
}

TEST(util_WavWriter, saveFile_flush0HalfOnce)
{
    test::ResetFatFsMockState();

    WavWriter<16>         writer;
    WavWriter<16>::Config cfg;
    cfg.samplerate    = 48000.0f;
    cfg.channels      = 1;
    cfg.bitspersample = 32;
    writer.Init(cfg);
    writer.OpenFile("flush0_once.wav");

    // kTransferSamps = 4 for WavWriter<16> in 32-bit mono, so this fills one half.
    const std::vector<float> first_half = {0.10f, 0.20f, 0.30f, 0.40f};
    SampleMono(writer, first_half);
    writer.SaveFile();

    EXPECT_EQ(ReadPcmPayloadAsS32Mono(), ToS32(first_half));
}

TEST(util_WavWriter, saveFile_flush0ThenSecondHalfTail)
{
    test::ResetFatFsMockState();

    WavWriter<16>         writer;
    WavWriter<16>::Config cfg;
    cfg.samplerate    = 48000.0f;
    cfg.channels      = 1;
    cfg.bitspersample = 32;
    writer.Init(cfg);
    writer.OpenFile("flush0_plus_tail.wav");

    const std::vector<float> first_half = {0.11f, 0.21f, 0.31f, 0.41f};
    const std::vector<float> second_tail = {0.51f, 0.61f};
    SampleMono(writer, first_half);
    SampleMono(writer, second_tail);
    writer.SaveFile();

    auto expected = ToS32(first_half);
    const auto tail = ToS32(second_tail);
    expected.insert(expected.end(), tail.begin(), tail.end());
    EXPECT_EQ(ReadPcmPayloadAsS32Mono(), expected);
}

TEST(util_WavWriter, saveFile_flush1HalfOnce)
{
    test::ResetFatFsMockState();

    WavWriter<16>         writer;
    WavWriter<16>::Config cfg;
    cfg.samplerate    = 48000.0f;
    cfg.channels      = 1;
    cfg.bitspersample = 32;
    writer.Init(cfg);
    writer.OpenFile("flush1_once.wav");

    const std::vector<float> first_half = {0.12f, 0.22f, 0.32f, 0.42f};
    const std::vector<float> second_half = {0.52f, 0.62f, 0.72f, 0.82f};

    SampleMono(writer, first_half);
    writer.Write(); // flush first half
    SampleMono(writer, second_half); // fill second half (FLUSH1 pending)
    writer.SaveFile();

    auto expected = ToS32(first_half);
    const auto second = ToS32(second_half);
    expected.insert(expected.end(), second.begin(), second.end());
    EXPECT_EQ(ReadPcmPayloadAsS32Mono(), expected);
}

TEST(util_WavWriter, saveFile_flush1ThenFirstHalfTail)
{
    test::ResetFatFsMockState();

    WavWriter<16>         writer;
    WavWriter<16>::Config cfg;
    cfg.samplerate    = 48000.0f;
    cfg.channels      = 1;
    cfg.bitspersample = 32;
    writer.Init(cfg);
    writer.OpenFile("flush1_plus_tail.wav");

    const std::vector<float> first_half = {0.13f, 0.23f, 0.33f, 0.43f};
    const std::vector<float> second_half = {0.53f, 0.63f, 0.73f, 0.83f};
    const std::vector<float> new_first_tail = {0.91f, 0.92f};

    SampleMono(writer, first_half);
    writer.Write(); // flush first half
    SampleMono(writer, second_half); // fill second half (FLUSH1 pending)
    SampleMono(writer, new_first_tail); // new tail in first half after wrap
    writer.SaveFile();

    auto expected = ToS32(first_half);
    const auto second = ToS32(second_half);
    const auto tail = ToS32(new_first_tail);
    expected.insert(expected.end(), second.begin(), second.end());
    expected.insert(expected.end(), tail.begin(), tail.end());
    EXPECT_EQ(ReadPcmPayloadAsS32Mono(), expected);
}

TEST(util_WavWriter, saveFile_s32StereoPartialTailOnce)
{
    test::ResetFatFsMockState();

    WavWriter<16>         writer;
    WavWriter<16>::Config cfg;
    cfg.samplerate    = 48000.0f;
    cfg.channels      = 2;
    cfg.bitspersample = 32;
    writer.Init(cfg);
    writer.OpenFile("s32_stereo_partial.wav");

    // One frame (< half-buffer for this config).
    const std::vector<std::array<float, 2>> frames = {{{0.10f, -0.10f}}};
    SampleStereo(writer, frames);
    writer.SaveFile();

    EXPECT_EQ(ReadPcmPayloadBytes(), ToPcmBytesS32Stereo(frames));
}

TEST(util_WavWriter, saveFile_s32StereoFlush0HalfOnce)
{
    test::ResetFatFsMockState();

    WavWriter<16>         writer;
    WavWriter<16>::Config cfg;
    cfg.samplerate    = 48000.0f;
    cfg.channels      = 2;
    cfg.bitspersample = 32;
    writer.Init(cfg);
    writer.OpenFile("s32_stereo_flush0.wav");

    // Half-buffer boundary for WavWriter<16>, 32-bit stereo = 2 frames.
    const std::vector<std::array<float, 2>> half_frames
        = {{{0.11f, -0.11f}}, {{0.21f, -0.21f}}};
    SampleStereo(writer, half_frames);
    writer.SaveFile();

    EXPECT_EQ(ReadPcmPayloadBytes(), ToPcmBytesS32Stereo(half_frames));
}

TEST(util_WavWriter, saveFile_s16MonoPartialTailOnce)
{
    test::ResetFatFsMockState();

    WavWriter<16>         writer;
    WavWriter<16>::Config cfg;
    cfg.samplerate    = 48000.0f;
    cfg.channels      = 1;
    cfg.bitspersample = 16;
    writer.Init(cfg);
    writer.OpenFile("s16_mono_partial.wav");

    const std::vector<float> samples = {0.14f, -0.24f, 0.34f, -0.44f, 0.54f};
    SampleMono(writer, samples);
    writer.SaveFile();

    EXPECT_EQ(ReadPcmPayloadBytes(), ToPcmBytesS16Mono(samples));
}

TEST(util_WavWriter, saveFile_s16MonoFlush0HalfOnce)
{
    test::ResetFatFsMockState();

    WavWriter<16>         writer;
    WavWriter<16>::Config cfg;
    cfg.samplerate    = 48000.0f;
    cfg.channels      = 1;
    cfg.bitspersample = 16;
    writer.Init(cfg);
    writer.OpenFile("s16_mono_flush0.wav");

    // Half-buffer boundary for WavWriter<16>, 16-bit mono = 8 samples.
    const std::vector<float> half_samples
        = {0.15f, 0.25f, 0.35f, 0.45f, 0.55f, 0.65f, 0.75f, 0.85f};
    SampleMono(writer, half_samples);
    writer.SaveFile();

    EXPECT_EQ(ReadPcmPayloadBytes(), ToPcmBytesS16Mono(half_samples));
}

TEST(util_WavWriter, saveFile_s16StereoCrossesIntoSecondHalf)
{
    test::ResetFatFsMockState();

    WavWriter<16>         writer;
    WavWriter<16>::Config cfg;
    cfg.samplerate    = 48000.0f;
    cfg.channels      = 2;
    cfg.bitspersample = 16;
    writer.Init(cfg);
    writer.OpenFile("s16_stereo_cross_half.wav");

    // 4 frames fill first half, next 2 frames are second-half tail.
    const std::vector<std::array<float, 2>> frames
        = {{{0.16f, -0.16f}},
           {{0.26f, -0.26f}},
           {{0.36f, -0.36f}},
           {{0.46f, -0.46f}},
           {{0.56f, -0.56f}},
           {{0.66f, -0.66f}}};
    SampleStereo(writer, frames);
    writer.SaveFile();

    EXPECT_EQ(ReadPcmPayloadBytes(), ToPcmBytesS16Stereo(frames));
}
