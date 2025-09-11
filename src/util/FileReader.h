#pragma once

#ifdef FILEIO_ENABLE_FATFS_READER
#include "ff.h"
#endif
#ifdef FILEIO_ENABLE_CSTDIO_READER
#include <cstdio>
#endif

namespace daisy
{
// Abstract reader interface (minimal). Provide concrete implementation for
// platform.
class IReader
{
  public:
    virtual ~IReader() = default;
    virtual size_t read(void*  dst,
                        size_t bytes)
        = 0;                                 // returns bytes actually read
    virtual bool     seek(uint32_t pos) = 0; // absolute seek from start
    virtual uint32_t position() const   = 0; // current absolute position
    virtual uint32_t size() const = 0; // total size if known (0 if unknown)
};

#ifdef FILEIO_ENABLE_FATFS_READER
class FileReader : public IReader
{
  public:
    explicit FileReader(FIL* f) : f_(f)
    {
        if(f_)
        {
            size_ = static_cast<uint32_t>(f_size(f_));
            f_rewind(f_);
        }
    }
    size_t read(void* dst, size_t bytes) override
    {
        /** This doesn't report any errors that may occur w/ the
     *  Filesystem.. that maybe something we want to add..
     */
        UINT br = 0;
        f_read(f_, dst, bytes, &br);
        return br;
    }
    bool seek(uint32_t pos) override
    {
        return f_lseek(f_, static_cast<FSIZE_t>(pos)) == FR_OK;
    }
    uint32_t position() const override
    {
        long p = f_tell(f_);
        return p < 0 ? 0u : static_cast<uint32_t>(p);
    }
    uint32_t size() const override { return size_; }

  private:
    FIL*     f_    = nullptr;
    uint32_t size_ = 0;
};
#endif

#ifdef FILEIO_ENABLE_CSTDIO_READER
class FileReader : public IReader
{
  public:
    explicit FileReader(FILE* f) : f_(f)
    {
        if(f_)
        {
            long pos = ftell(f_);
            if(pos < 0)
                pos = 0;
            if(fseek(f_, 0, SEEK_END) == 0)
            {
                long end = ftell(f_);
                if(end > 0)
                    size_ = static_cast<uint32_t>(end);
            }
            fseek(f_, pos, SEEK_SET);
        }
    }
    size_t read(void* dst, size_t bytes) override
    {
        return fread(dst, 1, bytes, f_);
    }
    bool seek(uint32_t pos) override
    {
        return fseek(f_, static_cast<long>(pos), SEEK_SET) == 0;
    }
    uint32_t position() const override
    {
        long p = ftell(f_);
        return p < 0 ? 0u : static_cast<uint32_t>(p);
    }
    uint32_t size() const override { return size_; }

  private:
    FILE*    f_    = nullptr;
    uint32_t size_ = 0;
};
#endif

} // namespace daisy