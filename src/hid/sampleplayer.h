#pragma once
#ifndef SAMPLE_PLAYER_H
#define SAMPLE_PLAYER_H

#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include "fatfs.h"
#include "hid/wavplayer.h"


// LONGTERM TODO:
// * Add Samplerate Adjustment
// * Add Stereo Support
// * Add 24/32-bit file suppport
// * Short File Playback (less than kStartBuffSize samps)

// LIBRARY ADD TODO:
// * Remove References to daisysp (just clamps and onepoles)

// For FATFS Fast Seek -- Not sure if this actually has much of an effect, 
//  but hypothetically would potentially help with jumping back to the 
//  beginning of very long files.
#define USE_FAST_SEEK
#define SZ_TBL 4


namespace daisy
{

static constexpr size_t kStartBuffSize = 4096;

struct AudioFileData
{
    inline int Init(std::string fname)
    {
        // Load Name
        size_t bytesread;
        //strcpy(name, fname);
        strcpy(name, fname.c_str());
        if(f_open(&fptr, name, (FA_OPEN_EXISTING | FA_READ)) == FR_OK)
        {
            // Populate the WAV Info
            if(f_read(&fptr,
                      (void *)&raw_data,
                      sizeof(WAV_FormatTypeDef),
                      &bytesread)
               != FR_OK)
            {
                return 1;
            }
            // Fill start buffer
            if(f_read(&fptr,
                      (void *)start_buff,
                      kStartBuffSize * sizeof(start_buff[0]),
                      &bytesread)
               != FR_OK)
            {
                return 1;
            }
#ifdef USE_FAST_SEEK
            fptr.cltbl = clmt;
            clmt[0] = SZ_TBL;
            int res = f_lseek(&fptr, CREATE_LINKMAP);
#endif
        }
        return 0;
    }
    inline int Open()
    {
        return f_open(&fptr, name, (FA_OPEN_EXISTING | FA_READ));
    }
    inline int      Close() { return f_close(&fptr); }
    inline size_t   DataStart() const { return sizeof(raw_data); }
    inline uint16_t Channels() const { return raw_data.NbrChannels; }
    inline uint16_t SampleRate() const { return raw_data.SampleRate; }
    inline uint16_t Size() const { return raw_data.SubCHunk2Size; }
    inline size_t   EndOfStartBuff()
    {
        return this->DataStart() + (sizeof(start_buff[0]) * kStartBuffSize);
    }
    inline bool eof(){
        return fptr.fptr >= fptr.obj.objsize;
    }
    FIL               fptr;
    WAV_FormatTypeDef raw_data;
    char              name[WAV_FILENAME_MAX]; /**< Wav filename */
    int16_t           start_buff[kStartBuffSize];
#ifdef USE_FAST_SEEK
    DWORD clmt[SZ_TBL];
#endif
};

struct SamplePlayerOptions
{
    enum
    {
        OPTION_QUANTIZE_PITCH,
        OPTION_GATED_PLAYBACK,
        OPTION_ADD_FADES,
        OPTION_LOCK_PITCH,
        OPTION_CHANGE_ON_LOOP,
        OPTION_LAST,
    };

    void Init()
    {
        const char *ostr[] = {
            "QUANTIZE_PITCH=",
            "GATED_PLAYBACK=",
            "ADD_FADES=",
            "LOCK_PITCH=",
            "CHANGE_ON_LOOP=",
        };
        // Defaults
        quantize_pitch                 = false;
        add_fades                      = true;
        gated_playback                 = false;
        lock_pitch                     = false;
        change_on_loop                 = false;
        options[OPTION_QUANTIZE_PITCH] = &quantize_pitch;
        options[OPTION_GATED_PLAYBACK] = &gated_playback;
        options[OPTION_ADD_FADES]      = &add_fades;
        options[OPTION_LOCK_PITCH]     = &lock_pitch;
        options[OPTION_CHANGE_ON_LOOP] = &change_on_loop;
        for(size_t i = 0; i < OPTION_LAST; i++)
        {
            strcpy(option_strings[i], ostr[i]);
        }
    }

    void Load()
    {
        DWORD    size;
        FRESULT  res;
        FILINFO  fno;
        DIR      dir;
        uint32_t bytesread;
        bool     found_options = false;
        char     buff[1024];
        char     option_file_name[256];
        char *   fn = NULL;
        // In same order as enum above
        memset(option_file_name, 0, 256);
        memset(buff, 0, 1024);
        if(f_opendir(&dir, SDPath) == FR_OK)
        {
            do
            {
                res = f_readdir(&dir, &fno);
                if(res != FR_OK || fno.fname[0] == 0)
                    break;
                if(fno.fname[0] == '.')
                    continue;
                fn = fno.fname;
                if((fno.fattrib & AM_DIR) == 0 && (fno.fattrib & AM_HID) == 0)
                {
                    if((strstr(fn, "options.txt")) || strstr(fn, "OPTIONS.TXT"))
                    {
                        strncpy(option_file_name, fn, 256);
                        size          = fno.fsize;
                        found_options = true;
                    }
                }
            } while(res == FR_OK && !found_options);
        }
        f_closedir(&dir);
        // Now Load
        if(found_options)
        {
            if(f_open(&SDFile, option_file_name, FA_OPEN_EXISTING | FA_READ)
               == FR_OK)
            {
                uint32_t totalbytes;
                totalbytes = 0;
                do
                {
                    f_read(&SDFile, (void *)buff, 1024, (UINT *)&bytesread);
                    totalbytes += bytesread;
                    // Parse for options
                    for(size_t i = 0; i < OPTION_LAST; i++)
                    {
                        //char* line_end;
                        char *toption, *tval, *line_end;
                        toption = tval = line_end = NULL;
                        uint32_t opt_len;
                        char     vbuff[8];
                        memset(vbuff, 0, 8);
                        toption = strstr(buff, option_strings[i]);
                        if(toption != NULL)
                        {
                            opt_len  = strlen(option_strings[i]);
                            line_end = strchr(toption, '\n');
                            // In case no \n at eof
                            if(line_end)
                                tval = toption + opt_len;
                            else
                                tval = toption + strlen(toption);
                            strncpy(vbuff,
                                    tval,
                                    line_end ? line_end - tval
                                             : strlen(toption));
                            *options[i] = atoi(vbuff);
                        }
                    }
                } while(totalbytes < size);
            }
            f_close(&SDFile);
        }
    }

    bool  quantize_pitch, add_fades, gated_playback;
    bool  lock_pitch, change_on_loop;
    bool *options[OPTION_LAST];
    char  option_strings[OPTION_LAST][32];
};

template <size_t max_files, size_t max_buffer>
class SamplePlayer
{
  public:
    SamplePlayer() {}
    ~SamplePlayer() {}

    void Init(float samplerate)
    {
        led.pin.pin = 7;
        led.pin.port = DSY_GPIOF;
        led.mode = DSY_GPIO_MODE_OUTPUT_PP;
        dsy_gpio_init(&led);

        // First check for all .wav files, and add them to the list until its full
        // or there are no more. Only checks '/'
        real_sr_  = samplerate;
        file_sel_ = file_cnt_ = 0;
        playing_ = looping_ = false;
        speed_              = 1.0f;
        read_ptr_f_         = 0.f;
        playback_buff_      = buff_;
        playback_size_ = read_size_ = max_buffer / 2;
        options_.Init();
        gate_fade_dest_ = 1.f;
        dsy_fatfs_init();
        f_mount(&SDFatFS, SDPath, 1);
        options_.Load();
        if(options_.gated_playback)
            gate_fade_dest_ = 0.f;
        LoadAudioFiles();
        Prepare();
        ignore_file_change_ = false;
    }

    inline void SetFileSel(size_t sel) { sel_ = sel; }

    inline void Stop() { stop_flag_ = true; }

    inline SamplePlayerOptions *GetOptions() { return &options_; }

    void DebugLed(){
        for(int i = 0; i < 100000; i++){
            dsy_gpio_write(&led, true);
        }
        dsy_gpio_write(&led, false);
    }

    inline int Open(size_t sel)
    {
        if(sel != file_sel_)
        {
            file_sel_ = sel < file_cnt_ ? sel : file_cnt_ - 1;
            return 0;
        }
        return 0;
    }

    int Close() { return f_close(&SDFile); }

    float mute_ = 1.f;

    inline float Stream()
    {
        size_t tndx;
        float  now, next, interp;
        float  ndx, frac;
        interp = 0.f;

        if(looping_ && !playing_ && !options_.gated_playback)
        {
            //playing_ = true;
            Restart();
            ignore_file_change_ = true;
        }

        if(prepare_eof_ == 0)
            mute_ = 1.f;
        if(playing_)
        {
            // Read from buffer and interpolate
            ndx  = read_ptr_f_;
            tndx = (size_t)ndx;
            frac = ndx - tndx;
            now    = (float)playback_buff_[tndx] / 32767.f;
            next   = (float)playback_buff_[(tndx + 1)] / 32767.f;
            interp = fclamp(
                (now + (next - now) * frac) * 0.9f, -1.f, 1.f);

            // Increment read ptr
            if(options_.lock_pitch)
            {
                ndx += (file_[file_sel_].SampleRate() / real_sr_);
            }
            else
            {
                ndx += PlaybackSpeed();
            }
            read_ptr_f_ = ndx;
            if((size_t)read_ptr_f_ + 1 >= playback_size_)
            {
                if(prepare_eof_ > 1){
                    mute_ = 0.f;
                }
                FlagBufferUpdate();
            }
        }
        // deal with the fades, and set output.
        fonepole(gate_fade_, gate_fade_dest_, 0.01f);
        if (stop_flag_ && gate_fade_ < 0.001f)
        {
            playing_ = false;
            stop_flag_ = false;
        }

        //get rid of the little pop at eof
        if(prepare_eof_ > 1 && playback_size_ - ndx < 50){
            start_fade_ = 0.f;
            return 0.f;
        }
        else if(options_.add_fades)
        {
            fonepole(retrig_fade_, retrig_fade_dest_, 0.02f);
            if(retrig_fade_ <= 0.0001f && deferred_restart_)
                Restart();
            if(start_fade_ < 1.f)
                start_fade_ += kFadeIncrement;
            else
                start_fade_ = 1.f;
            return (interp * gate_fade_) * start_fade_ * retrig_fade_ * mute_;
        }
        else
        {
            return (interp * gate_fade_) * mute_;
        }
    }

    int prepare_eof_;

    void RestartHelper(){
        if(prepare_eof_ > 2){
            if(looping_ && options_.add_fades)
            {
                Restart();
                if (!options_.change_on_loop)
                    ignore_file_change_ = true;
            }
            else
            {
                if(looping_)
                {
                    if(options_.change_on_loop)
                    {
                        Open(sel_);
                    }
                    if(options_.add_fades)
                    {
                        start_fade_      = 0.f;
                        start_fade_dest_ = 1.f;
                    }
                    RestartInternal();
                }
                else
                {
                    playing_ = false;
                    JumpToStart();
                }
            }
            prepare_eof_ = 0;
        }
    }

    dsy_gpio led;

    // Forced -O0 because for some reason streaming stops when optimization is on.
    // inline void __attribute__((optimize("-O0"))) Prepare()
    inline void Prepare()
    {
        RestartHelper();

        if(stop_flag_)
        {
            gate_fade_dest_ = 0.f;
            //stop_flag_      = false;
        }
        if(buff_state_ != BUFFER_STATE_IDLE)
        {
            size_t         offset, bytesread, rxsize;
            AudioFileData *f;
            f = &file_[file_sel_];
            if(buff_state_ == BUFFER_STATE_PREPARE_BOTH)
            {
                f_lseek(&f->fptr, f->EndOfStartBuff());
            }
            rxsize = read_size_ * sizeof(buff_[0]);
            offset = buff_state_ == BUFFER_STATE_PREPARE_1 ? read_size_ : 0;
            f_read(&f->fptr, &buff_[offset], rxsize, &bytesread);
            buff_state_ = BUFFER_STATE_IDLE;

            if(prepare_eof_ == 1){
                int fsize = f_size(&f->fptr) / 2;
                if ((size_t)fsize <= kStartBuffSize){
                    playback_size_ = kStartBuffSize - fsize;
                }
                else{
                    fsize -= kStartBuffSize;
                    playback_size_ = fsize % read_size_;
                }
            }

            // Check near end of file (only when fades are used)
            if(f_eof(&f->fptr))
            {
                prepare_eof_++;
            }
        }
    }

    inline void Restart()
    {
        prepare_eof_ = 0;
        if(options_.add_fades && !deferred_restart_)
        {
            deferred_restart_ = true;
            retrig_fade_dest_ = 0.f;
        }
        else
        {
            deferred_restart_   = false;

            if (!ignore_file_change_)
                Open(sel_);
            else
                ignore_file_change_ = false;
            
            read_ptr_f_    = 0.f;
            playback_buff_ = file_[file_sel_].start_buff;
            playback_size_ = kStartBuffSize;

            playing_        = true;
            gate_fade_dest_ = 1.f;
            buff_state_     = BUFFER_STATE_PREPARE_BOTH;
            // Start Fade
            start_fade_       = 0.f;
            start_fade_dest_  = 1.f;
            retrig_fade_      = 1.f;
            retrig_fade_dest_ = 1.f;
        }
    }

    inline void JumpToStart()
    {
        f_lseek(&file_[file_sel_].fptr, file_[file_sel_].EndOfStartBuff());
    }

    inline void RestartInternal()
    {
        //f_lseek(&file_[file_sel_].fptr, file_[file_sel_].DataStart());
        retrig_fade_dest_ = 1.f;
        retrig_fade_      = 1.f;
        if (!options_.change_on_loop)
            ignore_file_change_ = true;
        Restart();

    }

    inline void SetLooping(bool loop) { looping_ = loop; }

    inline bool GetLooping() const { return looping_; }

    inline size_t GetNumberFiles() const { return file_cnt_; }

    inline size_t GetCurrentFile() const { return file_sel_; }

    inline void SetPlaybackSpeed(float speed) { speed_ = speed; }

    inline float GetPlaybackSpeed() const { return speed_; }

  private:
    inline void fonepole(float &out, float in, float coeff)
    {
        out += coeff * (in - out);
    }

    inline float fclamp(float in, float min, float max)
    {
        return fmin(fmax(in, min), max);
    }

    enum BufferState
    {
        BUFFER_STATE_IDLE,
        BUFFER_STATE_PREPARE_0,
        BUFFER_STATE_PREPARE_1,
        BUFFER_STATE_PREPARE_BOTH,
    };

    void FlagBufferUpdate()
    {
        size_t prev_playback_size = playback_size_;
        if(playback_buff_ == buff_)
        {
            buff_state_    = BUFFER_STATE_PREPARE_0;
            playback_buff_ = buff_ + read_size_;
        }
        else if(playback_buff_ == buff_ + read_size_)
        {
            buff_state_    = BUFFER_STATE_PREPARE_1;
            playback_buff_ = buff_;
        }
        else
        {
            // Coming from start buffer
            buff_state_    = BUFFER_STATE_PREPARE_1;
            playback_buff_ = buff_;
        }

        playback_size_ = read_size_;
        read_ptr_f_ -= prev_playback_size;
    }

    inline float PlaybackSpeed()
    {
        float out, lout;
        float temp, hyst; //quant
        float semitone, prev_semitone;
        if(options_.quantize_pitch)
        {
            // do some crazy shit (taken from og play firmware)
            semitone      = (12.f * 5.f) * speed_;
            prev_semitone = (12.f * 5.f) * prev_speed_;
            temp          = (uint16_t)semitone;
            hyst          = semitone - prev_semitone > 0.f ? -.3f : .3f;
            temp          = (int32_t)(semitone + 0.5f + hyst);
            lout          = temp / (12.f * 5.f);
            prev_speed_   = speed_;
        }
        else
        {
            lout = speed_;
        }
        // rescale to approriate range (-3 octaves -> +2 octaves)
        out = expf(lout * (kSpeedLogMax - kSpeedLogMin) + kSpeedLogMin)
              * kSpeedScalar;

        return out;
    }

    void LoadAudioFiles()
    {
        FRESULT     result = FR_OK;
        FILINFO     fno;
        DIR         dir;
        char *      fn;
        std::string filename[max_files];
        // Open Dir and scan for files.
        if(f_opendir(&dir, SDPath) != FR_OK)
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
            if(fno.fattrib & (AM_HID | AM_DIR) || fno.fname[0] == '.')
                continue;
            // Now we'll check if its .wav and add to the list.
            fn = fno.fname;
            if(file_cnt_ < max_files - 1)
            {
                if(strstr(fn, ".wav") || strstr(fn, ".WAV"))
                {
                    filename[file_cnt_] = std::string(fn);
                    file_cnt_++;
                }
            }
            else
            {
                break;
            }
        } while(result == FR_OK);
        f_closedir(&dir);

        // Sort list alphabetically
        // The Array is sorted with all empty strings first
        // So we need to offset i by (max_files - file_cnt)
        std::sort(std::begin(filename), std::end(filename));
        for(size_t i = 0; i < file_cnt_; i++)
        {
            // Loads WavInfo and Start Buffer
            file_[i].Init(filename[(max_files - file_cnt_) + i]);
        }
    }

    // Constants
    static constexpr float kSpeedLogMin   = 0.f; // log(1)
    static constexpr float kSpeedLogMax   = logf(32.f);
    static constexpr float kSpeedScalar   = 0.125f;
    static constexpr float kFadeIncrement = 0.00104f * 4.f; // fast fade

    // Data
    AudioFileData       file_[max_files];
    SamplePlayerOptions options_;
    size_t              file_cnt_, file_sel_, sel_;
    float               real_sr_;

    // Buffer
    BufferState buff_state_;
    int16_t *   playback_buff_;
    int16_t     buff_[max_buffer];

    // Size and playback
    size_t playback_size_, read_size_;
    float  read_ptr_f_;
    bool   looping_, playing_;
    bool   stop_flag_;
    float  speed_, prev_speed_;

    // Fades
    float gate_fade_dest_, gate_fade_;     // log
    float start_fade_dest_, start_fade_;   // lin
    float retrig_fade_dest_, retrig_fade_; // short log
    bool  deferred_restart_;

    // Only for looping + add_fades option
    bool ignore_file_change_;
};


} // namespace play

#endif