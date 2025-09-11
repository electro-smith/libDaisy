#pragma once

#include "ff.h"
#include "WavParser.h"

class FileReader : public wav::IReader
{
  public:
    explicit FileReader(FIL *f) : f_(f)
    {
        if(f_)
        {
            size_ = static_cast<uint32_t>(f_size(f_));
            f_rewind(f_);
        }
    }
    size_t read(void *dst, size_t bytes) override
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
    FIL     *f_    = nullptr;
    uint32_t size_ = 0;
};