/* Current Limitations:
- 1x Playback speed only
- 16-bit, mono files only (otherwise fun weirdness can happen).
- Only 1 file playing back at a time.
- Not sure how this would interfere with trying to use the SDCard/FatFs outside of
this module. However, by using the extern'd SDFile, etc. I think that would break things.
*/
#pragma once
#ifndef DSY_POLYPHONIC_SAMPLE_PLAYER_H
#define DSY_POlYPHONIC_SAMPLE_PLAYER_H /**< Macro */
#include "daisy_core.h"
#include "util/wav_format.h"
#include "hid/wavplayer.h"

/** @file hid_wavplayer.h */

namespace daisy
{
/* 
TODO:
- 
*/


template <size_t max_files, size_t max_buffer, size_t max_polyphony>
class PolyphonicSamplePlayer
{
  public:
    PolyphonicSamplePlayer() {}
    ~PolyphonicSamplePlayer() {}

    /** Initializes the WavPlayer, loading up to max_files of wav files from an SD Card. */
    void Init(float sr) {
      for(int i = 0; i < max_polyphony; i++){
        players[i].Init();
      }
    }

    /** Opens the file at index sel for reading.
    \param sel File to open
     */
    int Open(size_t sel, uint16_t idx){
      if(idx < max_polyphony){
        players[idx].Open(sel);
      }
      return 0;
    }

    void SetFileSel(size_t sel, uint16_t idx){
      if(idx < max_polyphony){
        // players[idx].SetFileSel(sel);
      }
    }

    /** Closes whatever file is currently open.
    \return &
     */
    int Close(uint16_t idx){
      if(idx < max_polyphony){
        return players[idx].Close();
      }
    }

    int CloseAll(){
      for(int i = 0; i < max_polyphony; i++){
        int status = players[i].Close();
        
        if(status != 0){ //FR_OK
          return status;
        }
      }
    }

    float Stream(uint16_t idx){
      if(idx < max_polyphony){
        return (float)players[idx].Stream() / 32767.f;
      }
      return 0.f;
    }

    /** \return The next sample if playing, otherwise returns 0 */
    float StreamAll(){
      float sum = 0;

      for(int i = 0; i < max_polyphony; i++){
        sum += Stream(i);
      }

      return sum / (float)max_polyphony;
    }

    /** Collects buffer for playback when needed. */
    void Prepare() {
      for(int i = 0; i < max_polyphony; i++){
        players[i].Prepare();
      }
    }

    /** Resets the playback position to the beginning of the file immediately */
    void Restart(uint16_t idx){
      if (idx < max_polyphony){
        players[idx].Restart();
      }
    }

    void RestartAll(){
      for(int i = 0; i < max_polyphony; i++){
        Restart(i);
      }
    }

    /** Sets whether or not the current file will repeat after completing playback. 
    \param loop To loop or not to loop.
    */
    inline void SetLooping(bool loop, uint16_t idx) { 
      if (idx < max_polyphony){
        players[idx].SetLooping(loop);
      }
    }

    inline void SetLoopingAll(bool loop){
      for(int i = 0; i < max_polyphony; i++){
        SetLooping(loop, i);
      }
    }

    // /** \return Whether the WavPlayer is looping or not. */
    // inline bool GetLooping() const { return looping_; }

    // /** \return The number of files loaded by the WavPlayer */
    // inline size_t GetNumberFiles() const { return file_cnt_; }

    // /** \return currently selected file.*/
    // inline size_t GetCurrentFile() const { return file_sel_; }

  private:
    // SamplePlayer<max_files, max_buffer> players[max_polyphony];
    WavPlayer players[max_polyphony];
  
 };

} // namespace daisy

#endif