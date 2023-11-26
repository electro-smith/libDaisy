#pragma once
#include "fatfs.h"
#include "util/wav_format.h"
namespace daisy
{
/** Loads a bank of wavetables into memory. 
 ** Pointers to the start of each waveform will be provided, 
 ** but the user can do whatever they want with the data once
 ** it's imported. 
 **
 ** A internal 4kB workspace is used for reading from the file, and conveting to the correct memory location. 
 ** */
class WaveTableLoader
{
  public:
    enum class Result
    {
        OK,
        ERR_TABLE_INFO_OVERFLOW,
        ERR_FILE_READ,
        ERR_GENERIC,
    };
    WaveTableLoader() {}
    ~WaveTableLoader() {}

    /** Initializes the Loader */
    void Init(float *mem, size_t mem_size);

    /** Sets the size of the tables to allow access to the specific waveforms */
    Result SetWaveTableInfo(size_t samps, size_t count);

    /** Opens and loads the file 
     ** The data will be converted from its original type to float
     ** And the wavheader data will be stored internally to the class, 
     ** but will not be stored in the user-provided buffer.
     **
     ** Currently only 16-bit and 32-bit data is supported.
     ** The importer also assumes data is mono so stereo data will be loaded as-is 
     ** (i.e. interleaved)
     ** */
    Result Import(const char *filename);

    /** Returns pointer to specific table start or nullptr if invalid idx */
    float *GetTable(size_t idx);

  private:
    static constexpr int kWorkspaceSize = 1024;
    float *              buf_;
    size_t               buf_size_;
    WAV_FormatTypeDef    header_;
    size_t               samps_per_table_;
    size_t               num_tables_;
    int32_t              workspace[kWorkspaceSize];
    FIL                  fp_;
};

} // namespace daisy
