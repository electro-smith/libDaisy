/* Current Limitations:
- 1x Playback speed only
- 16-bit, mono files only (otherwise fun weirdness can happen).
- Only 1 file playing back at a time.
- Not sure how this would interfere with trying to use the SDCard/FatFs outside of
this module. However, by using the extern'd SDFile, etc. I think that would break things.
*/
#pragma once
#ifndef DSY_WAVPLAYER_H
#define DSY_WAVPLAYER_H /**< Macro */
#include "daisy_core.h"
#include "util/wav_format.h"
#include "ff.h"

#define WAV_FILENAME_MAX \
    256 /**< Maximum LFN (set to same in FatFs (ffconf.h) */

namespace daisy
{
// TODO: add bitrate, samplerate, length, etc.
/** Struct containing details of Wav File. */
struct WavFileInfo
{
    WAV_FormatTypeDef raw_data;               /**< Raw wav data */
    char              name[WAV_FILENAME_MAX]; /**< Wav filename */
};

/* 
TODO:
- Make template-y to reduce memory usage.
*/


/** Wav Player that will load .wav files from an SD Card,
and then provide a method of accessing the samples with
double-buffering. */
class WavPlayer
{
  public:
    WavPlayer() {}
    ~WavPlayer() {}

    /** Initializes the WavPlayer, loading up to max_files of wav files from an SD Card. */
    void Init(const char* search_path);

    /** Opens the file at index sel for reading.
    \param sel File to open
     */
    int Open(size_t sel);

    /** Closes whatever file is currently open.
    \return &
     */
    int Close();

    /** \return The next sample if playing, otherwise returns 0 */
    int16_t Stream();

    /** Collects buffer for playback when needed. */
    void Prepare();

    /** Resets the playback position to the beginning of the file immediately */
    void Restart();

    /** Sets whether or not the current file will repeat after completing playback. 
    \param loop To loop or not to loop.
    */
    inline void SetLooping(bool loop) { looping_ = loop; }

    /** \return Whether the WavPlayer is looping or not. */
    inline bool GetLooping() const { return looping_; }

    /** \return The number of files loaded by the WavPlayer */
    inline size_t GetNumberFiles() const { return file_cnt_; }

    /** \return currently selected file.*/
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
    static constexpr size_t kBufferSize = 4096;
    WavFileInfo             file_info_[kMaxFiles];
    size_t                  file_cnt_, file_sel_;
    BufferState             buff_state_;
    int16_t                 buff_[kBufferSize];
    size_t                  read_ptr_;
    bool                    looping_, playing_;
    FIL                     fil_;
};

} // namespace daisy

#endif
