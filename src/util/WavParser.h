#pragma once

// Minimal, allocation-free WAV (RIFF) parser suitable for embedded (e.g.,
// STM32). Supports canonical PCM / IEEE float WAV, handles JUNK and unknown
// chunks by skipping.
// Does not load sample data; records data offset & length
// so caller can stream.
//
// Limitations / Assumptions:
// - Little-endian host or platform where manual LE decoding is used (safe on
// STM32/Cortex-M).
// - No dynamic allocation; fixed maximum number of metadata entries.
// - Ignores extensible format extra fields beyond what is necessary for basic
// parsing.
// - Caller provides an abstract Reader (seek + read) so this can work with
// FatFS, cstdio, raw data, etc.
//
// Typical usage:
//   FileReader fr(fopen("file.wav", "rb"));
//   WavParser parser;
//   if(parser.parse(fr)) {
//      // Use parser.info() to get format, sampleRate, etc.
//      // Use parser.dataOffset(), parser.dataSize() to stream audio.
//   }
//

#include <cstddef>
#include <cstdint>
#include "FileReader.h"

namespace daisy
{
struct WavFormatInfo
{
    uint16_t audioFormat   = 0; // 1 = PCM, 3 = IEEE float, 0xFFFE = extensible
    uint16_t numChannels   = 0;
    uint32_t sampleRate    = 0;
    uint32_t byteRate      = 0;
    uint16_t blockAlign    = 0;
    uint16_t bitsPerSample = 0;
    // For extensible (0xFFFE)
    uint16_t validBitsPerSample = 0; // if provided
    uint32_t channelMask        = 0; // if provided
    uint16_t subFormat = 0; // wFormatTag of the sub-format GUID (first 2 bytes)
};

struct MetadataEntry
{
    uint32_t fourcc = 0; // chunk id
    uint32_t size   = 0; // payload size (before padding)
    uint32_t offset = 0; // file offset of chunk data
};

// Utility to form a FourCC constant at compile time: FCC("RIFF") not constexpr
// in pre-C++20 easily.
constexpr uint32_t make_fourcc(char a, char b, char c, char d)
{
    return (uint32_t(uint8_t(a))) | (uint32_t(uint8_t(b)) << 8)
           | (uint32_t(uint8_t(c)) << 16) | (uint32_t(uint8_t(d)) << 24);
}


class WavParser
{
  public:
    static constexpr uint32_t FOURCC_RIFF = make_fourcc('R', 'I', 'F', 'F');
    static constexpr uint32_t FOURCC_WAVE = make_fourcc('W', 'A', 'V', 'E');
    static constexpr uint32_t FOURCC_FMT  = make_fourcc('f', 'm', 't', ' ');
    static constexpr uint32_t FOURCC_DATA = make_fourcc('d', 'a', 't', 'a');
    static constexpr uint32_t FOURCC_JUNK = make_fourcc('J', 'U', 'N', 'K');
    static constexpr uint32_t FOURCC_FACT = make_fourcc('f', 'a', 'c', 't');
    static constexpr uint32_t FOURCC_LIST = make_fourcc('L', 'I', 'S', 'T');
    static constexpr uint32_t FOURCC_INFO = make_fourcc('I', 'N', 'F', 'O');

    static constexpr int MAX_METADATA_CHUNKS = 16; // tunable

    WavParser() = default;

    bool parse(IReader& r)
    {
        reset();
        if(!read_riff_header(r))
            return false;
        while(r.position() + 8 <= fileSize_)
        {
            ChunkHeader ch;
            if(!read_chunk_header(r, ch))
                return false;
            if(ch.id == FOURCC_FMT)
            {
                if(!parse_fmt_chunk(r, ch))
                    return false;
            }
            else if(ch.id == FOURCC_DATA)
            {
                dataOffset_ = r.position();
                dataSize_   = ch.size;
                // Skip data (we only record offset). Allow early break if we've got
                // fmt.
                if(!skip_chunk_payload(r, ch.size))
                    return false;
                haveData_ = true;
            }
            else
            {
                // Store metadata if room
                if(metadataCount_ < MAX_METADATA_CHUNKS)
                {
                    metadata_[metadataCount_].fourcc = ch.id;
                    metadata_[metadataCount_].size   = ch.size;
                    metadata_[metadataCount_].offset = r.position();
                    metadataCount_++;
                }
                if(!skip_chunk_payload(r, ch.size))
                    return false;
            }

            // Chunks are padded to even size
            if(ch.size & 1)
            {
                uint8_t pad;
                if(r.read(&pad, 1) != 1)
                    break;
            }

            if(haveFmt_ && haveData_)
                break; // parsed what we need
        }
        return haveFmt_ && haveData_;
    }

    const WavFormatInfo& info() const { return fmt_; }
    uint32_t             dataOffset() const { return dataOffset_; }
    uint32_t             dataSize() const { return dataSize_; }
    const MetadataEntry* metadata() const { return metadata_; }
    int                  metadataCount() const { return metadataCount_; }

  private:
    struct ChunkHeader
    {
        uint32_t id;
        uint32_t size;
    };

    void reset()
    {
        fmt_           = WavFormatInfo{};
        haveFmt_       = false;
        haveData_      = false;
        dataOffset_    = 0;
        dataSize_      = 0;
        metadataCount_ = 0;
        fileSize_      = 0;
    }

    static uint16_t rd_u16(const uint8_t* b)
    {
        return uint16_t(b[0]) | (uint16_t(b[1]) << 8);
    }
    static uint32_t rd_u32(const uint8_t* b)
    {
        return uint32_t(b[0]) | (uint32_t(b[1]) << 8) | (uint32_t(b[2]) << 16)
               | (uint32_t(b[3]) << 24);
    }

    bool read_exact(IReader& r, void* dst, size_t n)
    {
        return r.read(dst, n) == n;
    }

    bool read_riff_header(IReader& r)
    {
        uint8_t hdr[12];
        if(!read_exact(r, hdr, 12))
            return false;
        uint32_t riff           = rd_u32(hdr + 0);
        uint32_t fileSizeMinus8 = rd_u32(hdr + 4); // size of file - 8
        uint32_t wave           = rd_u32(hdr + 8);
        if(riff != FOURCC_RIFF || wave != FOURCC_WAVE)
            return false;
        fileSize_ = fileSizeMinus8 + 8; // nominal
        if(r.size() != 0)
            fileSize_ = r.size(); // trust reader if known
        return true;
    }

    bool read_chunk_header(IReader& r, ChunkHeader& ch)
    {
        uint8_t buf[8];
        if(!read_exact(r, buf, 8))
            return false;
        ch.id   = rd_u32(buf);
        ch.size = rd_u32(buf + 4);
        return true;
    }

    bool skip_chunk_payload(IReader& r, uint32_t sz)
    {
        // Seek ahead instead of reading to avoid buffer.
        uint32_t target = r.position() + sz;
        return r.seek(target);
    }

    bool parse_fmt_chunk(IReader& r, const ChunkHeader& ch)
    {
        if(ch.size < 16)
            return false;
        uint8_t core[16];
        if(!read_exact(r, core, 16))
            return false;
        fmt_.audioFormat   = rd_u16(core + 0);
        fmt_.numChannels   = rd_u16(core + 2);
        fmt_.sampleRate    = rd_u32(core + 4);
        fmt_.byteRate      = rd_u32(core + 8);
        fmt_.blockAlign    = rd_u16(core + 12);
        fmt_.bitsPerSample = rd_u16(core + 14);
        uint32_t consumed  = 16;

        if(fmt_.audioFormat != 1 && fmt_.audioFormat != 3
           && fmt_.audioFormat != 0xFFFE)
        {
            // unsupported basic format
            skip_rest_of_chunk(r, ch, consumed);
            return false;
        }

        if(ch.size > consumed)
        {
            // Read the remaining bytes (small), up to a cap we care about
            uint32_t remain = ch.size - consumed;
            // We'll process extension for extensible
            if(fmt_.audioFormat == 0xFFFE && remain >= 2)
            {
                uint8_t extSizeBuf[2];
                if(!read_exact(r, extSizeBuf, 2))
                    return false;
                consumed += 2;
                uint16_t extSize = rd_u16(extSizeBuf);
                if(extSize >= 22 && remain >= 2 + 22)
                { // extensible has at least 22 bytes after cbSize
                    uint8_t ext[22];
                    if(!read_exact(r, ext, 22))
                        return false;
                    consumed += 22;
                    fmt_.validBitsPerSample = rd_u16(ext + 0);
                    fmt_.channelMask        = rd_u32(ext + 2);
                    fmt_.subFormat          = rd_u16(
                        ext
                        + 6); // first two bytes of GUID contain the actual format tag
                    // skip any rest of ext
                    if(extSize > 22)
                    {
                        uint32_t skip = extSize - 22;
                        if(!skip_bytes(r, skip))
                            return false;
                        consumed += skip;
                    }
                }
                else
                {
                    // skip remainder if not long enough
                    if(!skip_bytes(r, remain - 2))
                        return false;   // we already read extSizeBuf
                    consumed = ch.size; // consumed all
                }
            }
            else
            {
                // skip any unneeded extended bytes for PCM / float
                if(!skip_bytes(r, remain))
                    return false;
                consumed = ch.size;
            }
        }
        haveFmt_ = true;
        return true;
    }

    bool skip_bytes(IReader& r, uint32_t count)
    {
        uint32_t target = r.position() + count;
        return r.seek(target);
    }

    bool
    skip_rest_of_chunk(IReader& r, const ChunkHeader& ch, uint32_t consumed)
    {
        if(consumed < ch.size)
            return skip_bytes(r, ch.size - consumed);
        return true;
    }

    WavFormatInfo fmt_{};
    bool          haveFmt_    = false;
    bool          haveData_   = false;
    uint32_t      dataOffset_ = 0;
    uint32_t      dataSize_   = 0;
    MetadataEntry metadata_[MAX_METADATA_CHUNKS];
    int           metadataCount_ = 0;
    uint32_t      fileSize_      = 0;
};

} // namespace daisy
