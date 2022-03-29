#ifndef WAV_COPY_GUARD
#define WAV_COPY_GUARD

#include "daisy.h"

class WavCopy {

  public:

    /** Attempts to copy the given wav file to QSPI flash
     *  \param qspi The QSPIHandle member of the hardware class
     *  \param filename The name of the file to copy
     *  \param address_offset Sets the address offset from the beginning of QSPI
     *  \returns Whether the operation was successful
     */
    bool CopyWav(daisy::QSPIHandle* qspi, const char* filename, uint32_t address_offset = daisy::System::kQspiBootloaderOffset);

    /** Copies all WAV file from the root directory to QSPI flash
     *  \param qspi The QSPIHandle member of the hardware class
     *  \param address_offset Sets the address offset from the beginning of QSPI for the first wav file
     *  \returns Whether the operation was successful
     */
    bool CopyWavs(daisy::QSPIHandle* qspi, uint32_t address_offset = daisy::System::kQspiBootloaderOffset);

    /** Gets an individual sample from the given wav file. Useful as a utility. This expects 
     *  mono, 16-bit integer data at 48kHz. If you know the format of your data, then don't
     *  worry; you can interpret it however you like.
     * 
     *  for verifying copied files.
     *  \param wav_index the index of the wav file for testing
     *  \param sample_index the index of the sample to pull
     */
    float GetSample(uint32_t wav_index, uint32_t sample_index);

    /** Gets the length of the given wav file.
     *  \param wav_index the index of the wav file for testing
     */
    uint32_t GetWavLen(uint32_t wav_index);

    /** Gets the total number of samples that were copied
     *  \returns the total number of samples copied
     */
    uint32_t GetTotalSamples() { return total_samples_; }

  private:

    enum Result
    {
      OK = 0,
      ERR
    };

    struct Sample
    {
      uint32_t address;
      uint32_t length;
    };

    Result InitFatfs();
    Result DeinitFatfs();

    void LoadWav(const char* filename);
    
    Result SearchWav();
    Result CreateLog();
    Result UpdateLog(const char *attempted_file, uint32_t address, uint32_t len, const char *message);

    static constexpr const char *WAV_LOG_NAME = "wav_info.txt";
    static constexpr uint32_t QSPI_INITIAL = 0x90000000U;
    static constexpr uint32_t QSPI_END = 0x90800000U;
    static constexpr uint32_t QSPI_LEN = 0x00800000U;
    static constexpr uint32_t SECTOR_SIZE = 65536;

    daisy::SdmmcHandler sd_;
    daisy::FatFSInterface fsi_;
    FIL file_;

    uint32_t current_qspi_offset_;
    // NOTE -- this must be a power of two below 65536
    static constexpr uint32_t DATA_BUFFER_LEN = 4096;
    uint8_t data_buffer_[DATA_BUFFER_LEN];

    daisy::QSPIHandle* qspi_;

    static constexpr uint32_t MAX_SAMPLES = 32;
    Sample saved_samples_[MAX_SAMPLES];
    uint32_t total_samples_;

};

#endif // WAV_COPY_GUARD
