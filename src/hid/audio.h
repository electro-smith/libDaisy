#ifndef DSY_AUDIO_H
#define DSY_AUDIO_H /**< & */

#include "per/sai.h"

namespace daisy
{
class AudioHandle
{
  public:
    /** Manually configurable details about the Audio Engine */
    /** TODO: Figure out how to get samplerate in here. */
    struct Config
    {
        size_t                        blocksize;
        SaiHandle::Config::SampleRate samplerate;
    };

    enum class Result
    {
        OK,
        ERR,
    };

    /** Non-Interleaving Callback format. Both arrays arranged by float[chn][sample] */
    typedef void (*AudioCallback)(float** in, float** out, size_t size);

    /** Non-Interleaving Callback format. 
     ** audio is prepared as { L0, R0, L1, R1, . . . LN, RN }
     */
    typedef void (*InterleavingAudioCallback)(float* in,
                                              float* out,
                                              size_t size);

    AudioHandle() : pimpl_(nullptr) {}
    ~AudioHandle() {}

    AudioHandle(const AudioHandle& other) = default;
    AudioHandle& operator=(const AudioHandle& other) = default;

    /** Initializes audio to run using a single SAI configured in Stereo I2S mode. */
    Result Init(const Config& config, SaiHandle sai);

    /** Initializes audio to run using two SAI, each configured in Stereo I2S mode. */
    Result Init(const Config& config, SaiHandle sai1, SaiHandle sai2);

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
