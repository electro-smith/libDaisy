#include "WaveTableLoader.h"
#include "daisy_core.h"
namespace daisy
{
void WaveTableLoader::Init(float *mem, size_t mem_size)
{
    buf_             = mem;
    buf_size_        = mem_size;
    samps_per_table_ = 256;
    num_tables_      = 1;
}

WaveTableLoader::Result WaveTableLoader::SetWaveTableInfo(size_t samps,
                                                          size_t count)
{
    if(samps * count > buf_size_)
        return Result::ERR_TABLE_INFO_OVERFLOW;
    samps_per_table_ = samps;
    num_tables_      = count;
    return Result::OK;
}

WaveTableLoader::Result WaveTableLoader::Import(const char *filename)
{
    if(f_open(&fp_, filename, FA_READ | FA_OPEN_EXISTING) == FR_OK)
    {
        // First Grab the Wave header info
        unsigned int br;
        f_read(&fp_, &header_, sizeof(header_), &br);
        uint32_t wptr = 0;
        do
        {
            f_read(&fp_, workspace, kWorkspaceSize * sizeof(workspace[0]), &br);
            // Fill mem
            switch(header_.BitPerSample)
            {
                case 16:
                {
                    int16_t *wsp;
                    wsp = (int16_t *)workspace;
                    for(size_t i = 0; i < kWorkspaceSize * 2; i++)
                    {
                        buf_[wptr] = s162f(wsp[i]);
                        wptr++;
                    }
                }
                break;
                case 32:
                {
                    float *wsp;
                    wsp = (float *)workspace;
                    for(size_t i = 0; i < kWorkspaceSize; i++)
                    {
                        buf_[wptr] = wsp[i];
                    }
                }
                break;
                default: break;
            }
        } while(!f_eof(&fp_) || wptr <= buf_size_ - 1);
        f_close(&fp_);
    }
    else
    {
        return Result::ERR_FILE_READ;
    }
    return Result::OK;
}

/** Returns pointer to specific table start or nullptr if invalid idx */
float *WaveTableLoader::GetTable(size_t idx)
{
    return idx < num_tables_ ? &buf_[idx * samps_per_table_] : nullptr;
}
} // namespace daisy
