// WAV Copying
// Example of how to copy WAV file over to QSPI flash
//
// Setup:
// * Insert an SD card into a connected SDMMC receptacle
//

#include "daisy_seed.h"
#include "wav_copy_class.h"

using namespace daisy;

DaisySeed hw;

// This class encapsulates all the copying code
WavCopy copy;

int main(void)
{
    // Initialize the Daisy Seed
    hw.Init();

    // To copy a single file with a known file name,
    // simple call `CopyWav`
    copy.CopyWav(&hw.qspi, "my_wav.wav");

    // If you want to copy all the WAV files in the
    // SD card's root directory, then call `CopyWavs`
    // copy.CopyWavs(&hw.qspi);

    // Both of these methods will write a table indicating 
    // where each file was written and how long it is in bytes.

    // The LED will illuminate when the copying is complete. For
    // large files, this can take some time.
    hw.SetLed(true);

    while(1);
}