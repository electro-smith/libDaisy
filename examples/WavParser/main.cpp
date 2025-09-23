/** Simple example of WavParser class
 *
 *  Prints out some information about the WAV files on
 *  an attached SD card.
 *
 *  To run this:
 *  1. Put some WAV files on an SD Card
 *     (This program will look at the first four it finds).
 *  2. Program the Daisy with this example, and the SD Card connected.
 *  3. Connect to the Daisy via USB Serial
 *  4. A list of the files found with some audio info will be output
 */
#include "daisy_seed.h"

using namespace daisy;

static constexpr const size_t kMaxFiles = 4;

static DaisySeed            hw;
static SdmmcHandler         sdmmc;
static FatFSInterface       fsi;
static FileTable<kMaxFiles> file_table;
static FIL                  file;

int main(void)
{
    /** Initialize our hardware */
    hw.Init(true);
    hw.StartLog(true);

    /** SD Card / FatFS Interface Init */
    SdmmcHandler::Config sdcfg;
    sdcfg.Defaults();
    sdcfg.speed = SdmmcHandler::Speed::STANDARD;
    sdcfg.width = SdmmcHandler::BusWidth::BITS_1;
    sdmmc.Init(sdcfg);
    fsi.Init(FatFSInterface::Config::Media::MEDIA_SD);
    f_mount(&fsi.GetSDFileSystem(), "/", 1);

    /** We'll fill up the table with WAV files and
     *  then parse those, and present some info.
     */
    file_table.Fill("/", ".wav");


    if(file_table.GetNumFiles() > 0)
    {
        for(size_t i = 0; i < file_table.GetNumFiles(); i++)
        {
            auto sta = f_open(
                &file, file_table.GetFileName(i), (FA_OPEN_EXISTING | FA_READ));
            if(sta != FR_OK)
            {
                hw.PrintLine("Could not open: %s", file_table.GetFileName(i));
                continue;
            }
            FileReader reader(&file);
            WavParser  parser;
            if(!parser.parse(reader))
            {
                hw.PrintLine("Error parsing file: %s",
                             file_table.GetFileName(i));
                continue;
            }

            const auto& info = parser.info();
            hw.PrintLine("File Information: %s", file_table.GetFileName(i));
            hw.PrintLine("\tSample Rate:\t%d", info.sampleRate);
            hw.PrintLine("\tChannels:\t%d", info.numChannels);
            hw.PrintLine("\tBit Depth:\t%d", info.bitsPerSample);

            /** File Duration in seconds */
            size_t dur_samples
                = parser.dataSize()
                  / ((info.bitsPerSample / 8) * info.numChannels);
            float dur_seconds = static_cast<float>(dur_samples)
                                / static_cast<float>(info.sampleRate);
            float frac = dur_seconds - static_cast<int>(dur_seconds);
            hw.PrintLine("\tDuration (seconds):\t%d.%02d",
                         static_cast<size_t>(dur_seconds),
                         static_cast<size_t>(frac * 100.f));

            /** Number of metadata chunks */
            hw.PrintLine("\tMetaData Chunks:\t%d", parser.metadataCount());
        }
    }


    while(1) {}
}
