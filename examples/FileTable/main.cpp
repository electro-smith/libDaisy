/** Simple demonstration of WAV file playback
 *
 *  When the program starts, it will attempt to load, and start looping
 *  the file, "loop.wav".
 *
 *  The "loop.wav" file used here is included in the repo for convenience.
 *  The file is a 48kHz stereo, 16-bit sine wave at 440Hz -6dB
 *
 *  Any 16-bit WAV file can be used with this class, but sample-rate
 *  is not automatically adjusted for.
 *
 *  The included file was created with sox, using the following command:
 *  sox -n -r 48000 -b 16 -c 2 loop.wav synth 1 sine 440 gain -6
 */
#include "daisy_seed.h"

using namespace daisy;

static constexpr const size_t kMaxFiles = 32;

static DaisySeed                hw;
static SdmmcHandler             sdmmc;
static FatFSInterface           fsi;
static FileTable<kMaxFiles>     file_table;

int main(void)
{
    /** Initialize our hardware */
    hw.Init(true);

    /** SD Card / FatFS Interface Init */
    SdmmcHandler::Config sdcfg;
    sdcfg.Defaults();
    sdcfg.speed = SdmmcHandler::Speed::STANDARD;
    sdcfg.width = SdmmcHandler::BusWidth::BITS_1;
    sdmmc.Init(sdcfg);
    fsi.Init(FatFSInterface::Config::Media::MEDIA_SD);
    f_mount(&fsi.GetSDFileSystem(), "/", 1);

    /** Fill the table with any files found in the root directory
     *  This will only fill up to the number of files specified
     *  in the template parameter for the class.
     *
     *  Without a second argument, this will include any files.
     */
    file_table.Fill("/");

    /** Write a log file containing tab separated information about
     *  each file including the file name, position, size
     */
    file_table.WriteLog("file_table-all.txt");

    /** Empty the table of its contents */
    file_table.Clear();

    /** Instead, if we supply a second argument, we can limit
     *  the files to those with a specific sub-string at the
     *  end (like file extensions).
     */
    file_table.Fill("/", ".txt");
    file_table.WriteLog("file_table-text.txt");

    /** And we'll do the same again for WAV files. */
    file_table.Clear();
    file_table.Fill("/", ".wav");
    file_table.WriteLog("file_table-wav.txt");

    while(1)
    {
        /** Blink Slower in normal operation */
        hw.SetLed((System::GetNow() & 511) < 255);
    }
}
