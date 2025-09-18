/** Simple example of FileTable class
 *
 *  This demonstrates quickly indexing a subset of files on a disk.
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

    /** We can loop through the files, checking their sizes */
    size_t largest_size = 0;
    int slot = -1;
    for (size_t i = 0; i < file_table.GetNumFiles(); i++) {
      auto fsize = file_table.GetFileSize(i);
      if (fsize > largest_size)  {
        largest_size = fsize;
        slot = i;
      }
    }

    /** Print this once the device is connected to serial. */
    hw.StartLog(true);
    System::Delay(100);
    hw.PrintLine("File Info:");
    if (slot >= 0) {
      hw.PrintLine("The largest file at %d bytes is: %s",
                   file_table.GetFileSize(slot),
                   file_table.GetFileName(slot));
    } else {
      hw.PrintLine("No files found..");
    }

    while(1){}
}
