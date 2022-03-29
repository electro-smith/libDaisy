#include "wav_copy_class.h"
#include "fatfs.h"

using namespace daisy;

bool WavCopy::CopyWav(QSPIHandle* qspi, const char* filename, uint32_t address_offset)
{
  current_qspi_offset_ = address_offset;
  qspi_ = qspi;
  total_samples_ = 0;

  Result res = Result::OK;

  InitFatfs();
  // Mount Media
  if (f_mount(&fsi_.GetSDFileSystem(), fsi_.GetSDPath(), 1) == FR_OK)
  {
    CreateLog();
    LoadWav(filename);
  }
  else
    res = Result::ERR;

  DeinitFatfs();

  return res;
}

bool WavCopy::CopyWavs(QSPIHandle* qspi, uint32_t address_offset)
{
  current_qspi_offset_ = address_offset;
  qspi_ = qspi;
  total_samples_ = 0;

  InitFatfs();
  Result res = SearchWav();
  DeinitFatfs();

  return res == Result::OK;
}

float WavCopy::GetSample(uint32_t wav_index, uint32_t sample_index)
{
  if (wav_index < total_samples_)
  {
    uint32_t length = saved_samples_[wav_index].length / sizeof(int16_t);
    if (sample_index < length)
    {
      int16_t* sample_start = (int16_t*) saved_samples_[wav_index].address;
      return (float) *(sample_start + sample_index) / (float) 0x7FFF;
    }
  }
  return 0;
}

uint32_t WavCopy::GetWavLen(uint32_t wav_index)
{
  if (wav_index < total_samples_)
  {
    return saved_samples_[wav_index].length / sizeof(int16_t);
  }
  return 0;
}

WavCopy::Result WavCopy::InitFatfs()
{
  SdmmcHandler::Config sd_cfg;
  sd_cfg.Defaults();
  sd_.Init(sd_cfg);

  fsi_.Init(FatFSInterface::Config::MEDIA_SD);

  return Result::OK;
}

WavCopy::Result WavCopy::DeinitFatfs()
{
  fsi_.DeInit();
  return Result::OK;
}

void WavCopy::LoadWav(const char* filename)
{
  if (f_open(&file_, filename, FA_OPEN_EXISTING | FA_READ) != FR_OK)
  {
    // There's a small chance we'll encounter an error in opening the file
    f_close(&file_);
    UpdateLog(filename, 0, 0, "unable to open file");
    return;
  }

  // Write file data to QSPI
  UINT data_read;
  uint32_t initial_address = current_qspi_offset_ + QSPI_INITIAL;
  uint32_t total_data_len = 0;

  WAV_FormatTypeDef wav_header;

  // This will populate the wav_header struct with all the data we 
  // need to parse the wav file. Currently, the code doesn't actually use this header,
  // but you could write code that converts from the
  // several possible WAV storage types to int16_t or float for consistency.

  // It seems easier to simply ensure your WAV files are all in the same format, though.
  f_read(&file_, &wav_header, sizeof(wav_header), &data_read);

  if (data_read != sizeof(wav_header))
  {
    f_close(&file_);
    UpdateLog(filename, 0, 0, "malformed wav header");
    return;
  }

  bool exceeded_qspi_size = false;

  do
  {
    if (current_qspi_offset_ >= QSPI_LEN)
    {
      exceeded_qspi_size = true;
      break;
    }
    
    // The QSPI chip must be erased before any writes can be made,
    // and it can only be erased in certain sizes (4K, 32K, 64K, and full chip erase).
    // 64K is the fastest per byte without being too large, so that's what we'll use.
    if (current_qspi_offset_ % SECTOR_SIZE == 0)
    {
      qspi_->Erase(current_qspi_offset_, current_qspi_offset_ + SECTOR_SIZE);
    }
    f_read(&file_, data_buffer_, DATA_BUFFER_LEN, &data_read);

    qspi_->Write(current_qspi_offset_, data_read, data_buffer_);
    current_qspi_offset_ += data_read;
    total_data_len += data_read;
  } while (data_read == DATA_BUFFER_LEN);

  f_close(&file_);

  if (exceeded_qspi_size)
  {
    // This code will allow you to partially copy a wav file if it's too big
    UpdateLog(filename, initial_address, total_data_len, "File exceeded total QSPI length");
  }
  else
  {
    UpdateLog(filename, initial_address, total_data_len, "");
  }

  // This will simply allow us to test the file copying. It's expected that you'll
  // hardcode the wav addresses and lengths in your intended application using
  // the log file (wav_info.txt) that this code generates
  saved_samples_[total_samples_++] = {initial_address, total_data_len};

  // Now align qspi offset to next DATA_BUFFER_LEN byte chunk. This prevents
  // the wav files from becoming misaligned according to the QSPI's sectors,
  // making erasing and writing a bit easier. You would have to be more careful
  // if you want to maximize your storage space.
  int32_t alignment = current_qspi_offset_ % DATA_BUFFER_LEN;
  if (alignment)
    current_qspi_offset_ += DATA_BUFFER_LEN - alignment;
}

WavCopy::Result WavCopy::SearchWav()
{
  // Mount Media
  if (f_mount(&fsi_.GetSDFileSystem(), fsi_.GetSDPath(), 1) == FR_OK)
  {
    DIR dir;
    FILINFO info;
    FRESULT result = FR_OK;

    if (f_opendir(&dir, fsi_.GetSDPath()) != FR_OK)
    {
      // Abort if opening the root directory is unsuccessful
      return Result::ERR;
    }

    CreateLog();

    do
    {
      result = f_readdir(&dir, &info);
      // Exit if bad read or NULL fname
      if (result != FR_OK || info.fname[0] == 0)
        break;
      // Skip if its a directory or a hidden file.
      if (info.fattrib & (AM_HID | AM_DIR))
        continue;

      if (strstr(info.fname, ".wav") || strstr(info.fname, ".WAV"))
      {
        // If the file name contains .wav or .WAV, then we'll attempt to load it
        LoadWav(info.fname);
      }
    } while (result == FR_OK);

    f_closedir(&dir);

    return Result::OK;
  }

  return Result::ERR;
}

WavCopy::Result WavCopy::CreateLog()
{
  if (f_open(&file_, WAV_LOG_NAME, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
  {
    UINT data_written;
    UINT data_to_write = sprintf((char*) data_buffer_, "File,Address,Length,Notes\n");

    f_write(&file_, data_buffer_, data_to_write, &data_written);
    f_close(&file_);

    return Result::OK;
  }

  return Result::ERR;
}

WavCopy::Result WavCopy::UpdateLog(const char *attempted_file, uint32_t address, uint32_t len, const char *message)
{
  if (f_open(&file_, WAV_LOG_NAME, FA_WRITE | FA_OPEN_APPEND) == FR_OK)
  {
    UINT data_to_write;
    UINT data_written;

    data_to_write = sprintf(
      (char*) data_buffer_,
      "%s,0x%08lX,0x%08lX,%s\n",
      attempted_file,
      address,
      len,
      message
    );

    f_write(&file_, data_buffer_, data_to_write, &data_written);
    f_close(&file_);

    return Result::OK;
  }

  return Result::ERR;
}
