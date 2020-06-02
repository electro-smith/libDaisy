/** Wav Player that will load .wav files from an SD Card,
and then provide a method of accessing the samples with
double-buffering.
*/
//
/** Current Limitations:
- 1x Playback speed only
- 16-bit, mono files only (otherwise fun weirdness can happen).
- Only 1 file playing back at a time.
- Not sure how this would interfere with trying to use the SDCard/FatFs outside of
*/
/**        this module. However, by using the extern'd SDFile, etc. I think that would break things.
*/
//
//
#pragma once
#ifndef DSY_WAVPLAYER_H
#define DSY_WAVPLAYER_H
#include "daisy_core.h"
#include "util_wav_format.h"

#define WAV_FILENAME_MAX 256 // Maximum LFN (set to same in FatFs (ffconf.h)

namespace daisy
{
/** Struct containing details of Wav File.
*/
//
/** TODO: add bitrate, samplerate, length, etc.
*/
struct WavFileInfo
{
    WAV_FormatTypeDef raw_data;
    char              name[WAV_FILENAME_MAX];
};

/** Class for handling playback of WAV files.

TODO:
- Make template-y to reduce memory usage.
*/
//
class WavPlayer
{
  public:
    WavPlayer() {}
    ~WavPlayer() {}

    /** Initializes the WavPlayer, loading up to max_files of wav files
from an SD Card.
*/
    void Init();
    /** Opens the file at index sel for reading.
*/
    int Open(size_t sel);
    /** Closes whatever file is currently open.
*/
    int Close();
    /** Returns the next sample if playing, otherwise returns 0
*/
    int16_t Stream();
    /** Collects buffer for playback when needed.
*/
    void Prepare();
    /** Resets the playback position to the beginning of the file immediately
*/
    void Restart();

    /** Sets whether or not the current file will repeat after completing playback.
*/
    inline void SetLooping(bool loop) { looping_ = loop; }

    /** Returns whether the WavPlayer is looping or not.
*/
    inline bool GetLooping() const { return looping_; }

    /** Returns the number of files loaded by the WavPlayer
*/
    inline size_t GetNumberFiles() const { return file_cnt_; }

    /** Returns currently selected file.
*/
    inline size_t GetCurrentFile() const { return file_sel_; }

  private:
    enum BufferState
    {
        BUFFER_STATE_IDLE,
        BUFFER_STATE_PREPARE_0,
        BUFFER_STATE_PREPARE_1,
    };

    BufferState GetNextBuffState();

    static constexpr size_t kMaxFiles   = 8;
    static constexpr size_t kBufferSize = 512;
    WavFileInfo             file_info_[kMaxFiles];
    size_t                  file_cnt_, file_sel_;
    BufferState             buff_state_;
    int16_t                 buff_[kBufferSize];
    size_t                  read_ptr_;
    bool                    looping_, playing_;
};

} // namespace daisy

#endif