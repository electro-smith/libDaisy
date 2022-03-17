#ifndef DSY_AUDIO_H
#define DSY_AUDIO_H /**< & */

#include "per/sai.h"

namespace daisy
{
/** @brief Audio Engine Handle
 *  @ingroup audio 
 *  @details This class allows for higher level access to an audio engine.
 *           If you're using a SOM like the DaisySeed or DaisyPatchSM (or any 
 *            board that includes one of those objects) then the intialization
 *            is already taken  care of.
 *           If you're setting up your own custom hardware, or need to make changes
 *           you will have to take the following steps:
 *             1. Create and Initialize an SaiHandle or two depending on your requirements
 *             2. Initialize the Audio Handle with the desired settings and the Initialized SaiHandle
 *             3. If the connected codec requires special configuration or initialization, do so
 *             4. Write a callback method using either the AudioCallback or the InterleavingAudioCallback format
 *             5. Start the Audio using one of the StartAudio function
 */
class AudioHandle
{
  public:
    /** Manually configurable details about the Audio Engine */
    /** TODO: Figure out how to get samplerate in here. */
    struct Config
    {
        size_t                        blocksize;
        SaiHandle::Config::SampleRate samplerate;
        float                         postgain;
    };

    enum class Result
    {
        OK,
        ERR,
    };

    /** Non-Interleaving input buffer
     * Buffer arranged by float[chn][sample] 
     * const so that the user can't modify the input
     */
    typedef const float* const* InputBuffer;

    /** Non-Interleaving output buffer
     * Arranged by float[chn][sample] 
     */
    typedef float** OutputBuffer;

    /** Type for a Non-Interleaving audio callback 
     * Non-Interleaving audio callbacks in daisy will be of this type
     */
    typedef void (*AudioCallback)(InputBuffer  in,
                                  OutputBuffer out,
                                  size_t       size);

    /** Interleaving Input buffer
     ** audio is prepared as { L0, R0, L1, R1, . . . LN, RN }]
     ** this is const, as the user shouldn't modify it
    */
    typedef const float* InterleavingInputBuffer;

    /** Interleaving Output buffer 
     ** audio is prepared as { L0, R0, L1, R1, . . . LN, RN }
    */
    typedef float* InterleavingOutputBuffer;

    /** Interleaving Audio Callback 
     * Interleaving audio callbacks in daisy must be of this type
     */
    typedef void (*InterleavingAudioCallback)(InterleavingInputBuffer  in,
                                              InterleavingOutputBuffer out,
                                              size_t                   size);

    AudioHandle() : pimpl_(nullptr) {}
    ~AudioHandle() {}

    AudioHandle(const AudioHandle& other) = default;
    AudioHandle& operator=(const AudioHandle& other) = default;

    /** Initializes audio to run using a single SAI configured in Stereo I2S mode. */
    Result Init(const Config& config, SaiHandle sai);

    /** Initializes audio to run using two SAI, each configured in Stereo I2S mode. */
    Result Init(const Config& config, SaiHandle sai1, SaiHandle sai2);

    /** Stops and deinitializes audio. */
    Result DeInit();

    /** Returns the Global Configuration struct for the Audio */
    const Config& GetConfig() const;

    /** Returns the number of channels of audio.  
     **
     ** When using a single SAI this returns 2, when using two SAI it returns 4
     ** If no SAI is initialized this returns 0
     **
     ** Eventually when we add non-standard I2S for each SAI this will be work differently
     */
    size_t GetChannels() const;

    /** Returns the Samplerate as a float */
    float GetSampleRate();

    /** Sets the samplerate, and reinitializes the sai as needed. */
    Result SetSampleRate(SaiHandle::Config::SampleRate samplerate);

    /** Sets the block size after initialization, and updates the internal configuration struct.
     ** Get BlockSize and other details via the GetConfig 
     */
    Result SetBlockSize(size_t size);

    /** Sets the amount of gain adjustment to perform before and after callback.
     ** useful if the hardware has additional headroom, and the nominal value shouldn't be 1.0 
     ** 
     ** \param val Gain adjustment amount. The hardware will clip at the reciprical of this value. */
    Result SetPostGain(float val);

    /** Starts the Audio using the non-interleaving callback. */
    Result Start(AudioCallback callback);

    /** Starts the Audio using the interleaving callback. 
     ** For now only two channels are supported via this method. 
     */
    Result Start(InterleavingAudioCallback callback);

    /** Stop the Audio*/
    Result Stop();

    /** Immediatley changes the audio callback to the non-interleaving callback passed in. */
    Result ChangeCallback(AudioCallback callback);

    /** Immediatley changes the audio callback to the interleaving callback passed in. */
    Result ChangeCallback(InterleavingAudioCallback callback);


    class Impl;

  private:
    Impl* pimpl_;
};

} // namespace daisy

#endif
