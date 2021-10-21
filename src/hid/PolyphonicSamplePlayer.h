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
#include "hid/sampleplayer.h"

#define MAX_PLAYERS 1

/** @file hid_wavplayer.h */

namespace daisy
{
/* 
TODO:
- 
*/


/** */
class PolyphonicSamplePlayer
{
  public:
    PolyphonicSamplePlayer() {}
    ~PolyphonicSamplePlayer() {}

    /** Initializes the WavPlayer, loading up to max_files of wav files from an SD Card. */
    void Init(float sr) {
      for(int i = 0; i < MAX_PLAYERS; i++){
        players[i].Init(sr);
      }
    }

    /** Opens the file at index sel for reading.
    \param sel File to open
     */
    int Open(size_t sel, uint16_t idx){
      if(idx < MAX_PLAYERS){
        players[idx].Open(sel);
      }
      return 0;
    }

    /** Closes whatever file is currently open.
    \return &
     */
    int Close(uint16_t idx){
      if(idx < MAX_PLAYERS){
        return players[idx].Close();
      }
    }

    int CloseAll(){
      for(int i = 0; i < MAX_PLAYERS; i++){
        int status = players[i].Close();
        
        if(status != 0){ //FR_OK
          return status;
        }
      }
    }

    /** \return The next sample if playing, otherwise returns 0 */
    int16_t Stream(){
      int sum = 0;

      for(int i = 0; i < MAX_PLAYERS; i++){
        sum += players[i].Stream();
      }

      return sum / MAX_PLAYERS;
    }

    /** Collects buffer for playback when needed. */
    void Prepare() {
      for(int i = 0; i < MAX_PLAYERS; i++){
        players[i].Prepare();
      }
    }

    /** Resets the playback position to the beginning of the file immediately */
    void Restart(uint16_t idx){
      if (idx < MAX_PLAYERS){
        players[idx].Restart();
      }
    }

    void RestartAll(){
      for(int i = 0; i < MAX_PLAYERS; i++){
        Restart(i);
      }
    }

    /** Sets whether or not the current file will repeat after completing playback. 
    \param loop To loop or not to loop.
    */
    inline void SetLooping(bool loop, uint16_t idx) { 
      if (idx < MAX_PLAYERS){
        players[idx].SetLooping(loop);
      }
    }

    inline void SetLoopingAll(bool loop){
      for(int i = 0; i < MAX_PLAYERS; i++){
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
    SamplePlayer<32, 32767> players[MAX_PLAYERS];
  
 };

} // namespace daisy

#endif