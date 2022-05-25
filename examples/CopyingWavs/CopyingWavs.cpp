// WAV Copying
// Example of how to copy WAV file over to QSPI flash
//
// Setup:
// * Insert an SD card into a connected SDMMC receptacle
//
#include "daisy_seed.h"

using namespace daisy;

DaisySeed hw;

// These classes are necessary for interacting with FatFS
SdmmcHandler sd;
FatFSInterface fsi;
FIL file;

// Powers of two are best for this data buffer to keep QSPI
// writes aligned to the sectors.
#define DATA_BUFFER_LEN 4096
uint8_t data_buffer[DATA_BUFFER_LEN];

// Change this to the file you'd like to copy
#define FILE_NAME "my_wav.wav"

// You can find the information on the QSPI's length
// in the linker scripts, among other places
#define QSPI_LEN     0x00800000U

#define SECTOR_SIZE  65536

void copy_wav(const char* filename);

int main(void)
{
    // Initialize the Daisy Seed
    hw.Init();

    // Run the copy routine
    copy_wav(FILE_NAME);

    // The LED will illuminate when the copying is complete. For
    // large files, this can take some time.
    hw.SetLed(true);

    while(1);
}

void copy_wav(const char* filename)
{
    // Initialize the SDMMC interface and FatFS drivers
    SdmmcHandler::Config sd_cfg;
    sd_cfg.Defaults();
    sd.Init(sd_cfg);
    fsi.Init(FatFSInterface::Config::MEDIA_SD);

    if (f_mount(&fsi.GetSDFileSystem(), fsi.GetSDPath(), 1) != FR_OK)
    {
        // If some error is encountered in mounting the card, then simply return.
        return;
    }

    if (f_open(&file, filename, FA_OPEN_EXISTING | FA_READ) != FR_OK)
    {
        // If some error is encountered in opening the file, then simply return.
        f_close(&file);
        return;
    }

    UINT data_read;
    WAV_FormatTypeDef wav_header;

    // This will populate the wav_header struct with all the data we 
    // need to parse the wav file. Currently, the code doesn't actually use this header,
    // but you could write code that converts from the
    // several possible WAV storage types to int16_t or float for consistency.
    // It seems easier to simply ensure your WAV files are all in the same format, though.

    // You could also write this header along with the audio data if you'd like to
    // parse the wav file in your actual project.

    f_read(&file, &wav_header, sizeof(wav_header), &data_read);

    // This helps us keep track of the QSPI address to write to
    // between file data chunks. You can set its initial value to
    // whatever you want -- you'll just need to remember where that
    // is so your target application can load it from QSPI.
    uint32_t current_qspi_offset = 0;

    do
    {
        // This avoids attempted writes beyond the QSPI's address space
        if (current_qspi_offset >= QSPI_LEN)
            break;
        
        // The QSPI chip must be erased before any writes can be made,
        // and it can only be erased in certain sizes (4K, 32K, 64K, and full chip erase).
        // 64K is the fastest per byte without being too large, so that's what we'll use.
        if (current_qspi_offset % SECTOR_SIZE == 0)
        {
            hw.qspi.Erase(current_qspi_offset, current_qspi_offset + SECTOR_SIZE);
        }
        f_read(&file, data_buffer, DATA_BUFFER_LEN, &data_read);

        hw.qspi.Write(current_qspi_offset, data_read, data_buffer);
        current_qspi_offset += data_read;
    } while (data_read == DATA_BUFFER_LEN);

    f_close(&file);
}