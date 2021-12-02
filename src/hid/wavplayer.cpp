#include <cstring>
#include "hid/wavplayer.h"

using namespace daisy;

void WavPlayer::Init(const char *search_path)
{
    // First check for all .wav files, and add them to the list until its full or there are no more.
    // Only checks '/'
    FRESULT result = FR_OK;
    FILINFO fno;
    DIR     dir;
    char *  fn;
    file_sel_ = 0;
    file_cnt_ = 0;
    playing_  = true;
    looping_  = false;
    // Open Dir and scan for files.
    if(f_opendir(&dir, search_path) != FR_OK)
    {
        return;
    }
    do
    {
        result = f_readdir(&dir, &fno);
        // Exit if bad read or NULL fname
        if(result != FR_OK || fno.fname[0] == 0)
            break;
        // Skip if its a directory or a hidden file.
        if(fno.fattrib & (AM_HID | AM_DIR))
            continue;
        // Now we'll check if its .wav and add to the list.
        fn = fno.fname;
        if(file_cnt_ < kMaxFiles - 1)
        {
            if(strstr(fn, ".wav") || strstr(fn, ".WAV"))
            {
                strcpy(file_info_[file_cnt_].name, search_path);
                strcat(file_info_[file_cnt_].name, fn);
                file_cnt_++;
                // For now lets break anyway to test.
                //                break;
            }
        }
        else
        {
            break;
        }
    } while(result == FR_OK);
    f_closedir(&dir);
    // Now we'll go through each file and load the WavInfo.
    for(size_t i = 0; i < file_cnt_; i++)
    {
        size_t bytesread;
        if(f_open(&fil_, file_info_[i].name, (FA_OPEN_EXISTING | FA_READ))
           == FR_OK)
        {
            // Populate the WAV Info
            if(f_read(&fil_,
                      (void *)&file_info_[i].raw_data,
                      sizeof(WAV_FormatTypeDef),
                      &bytesread)
               != FR_OK)
            {
                // Maybe add return type
                return;
            }
            f_close(&fil_);
        }
    }
    // fill buffer with first file preemptively.
    buff_state_ = BUFFER_STATE_PREPARE_0;
    Open(0);
    Prepare();
    read_ptr_ = 0;
}


int WavPlayer::Open(size_t sel)
{
    if(sel != file_sel_)
    {
        f_close(&fil_);
        file_sel_ = sel < file_cnt_ ? sel : file_cnt_ - 1;
    }
    // Set Buffer Position
    return f_open(
        &fil_, file_info_[file_sel_].name, (FA_OPEN_EXISTING | FA_READ));
}

int WavPlayer::Close()
{
    return f_close(&fil_);
}

int16_t WavPlayer::Stream()
{
    int16_t samp;
    if(playing_)
    {
        samp = buff_[read_ptr_];
        // Increment rpo
        read_ptr_ = (read_ptr_ + 1) % kBufferSize;
        if(read_ptr_ == 0)
            buff_state_ = BUFFER_STATE_PREPARE_1;
        else if(read_ptr_ == kBufferSize / 2)
            buff_state_ = BUFFER_STATE_PREPARE_0;
    }
    else
    {
        samp = 0;
        if(looping_)
            playing_ = true;
    }
    return samp;
}

void WavPlayer::Prepare()
{
    if(buff_state_ != BUFFER_STATE_IDLE)
    {
        size_t offset, bytesread, rxsize;
        bytesread = 0;
        rxsize    = (kBufferSize / 2) * sizeof(buff_[0]);
        offset    = buff_state_ == BUFFER_STATE_PREPARE_1 ? kBufferSize / 2 : 0;
        f_read(&fil_, &buff_[offset], rxsize, &bytesread);
        if(bytesread < rxsize || f_eof(&fil_))
        {
            if(looping_)
            {
                Restart();
                f_read(&fil_,
                       &buff_[offset + (bytesread / 2)],
                       rxsize - bytesread,
                       &bytesread);
            }
            else
            {
                playing_ = false;
            }
        }
        buff_state_ = BUFFER_STATE_IDLE;
    }
}

void WavPlayer::Restart()
{
    playing_ = true;
    f_lseek(&fil_,
            sizeof(WAV_FormatTypeDef)
                + file_info_[file_sel_].raw_data.SubChunk1Size);
}

WavPlayer::BufferState WavPlayer::GetNextBuffState()
{
    size_t next_samp;
    next_samp = (read_ptr_ + 1) % kBufferSize;
    if(next_samp < kBufferSize / 2)
    {
        return BUFFER_STATE_PREPARE_1;
    }
    else
    {
        return BUFFER_STATE_PREPARE_0;
    }
}
