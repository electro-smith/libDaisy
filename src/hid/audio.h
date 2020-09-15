#ifndef DSY_AUDIO_H
#define DSY_AUDIO_H /**< & */

#include "per/sai.h"

namespace daisy
{

class AudioHandle
{
  public:
    struct Config
    {
        size_t blocksize;
    };

    enum class Result
    {
        OK,
        ERR,
    };

    AudioHandle() : pimpl_(nullptr) {}
    ~AudioHandle() {}

    AudioHandle(const AudioHandle& other) = default;
    AudioHandle& operator=(const AudioHandle& other) = default;

    Result Init(const Config& config, SaiHandle sai);

    // TODO figure out multi-sai situation (i.e. Daisy Patch)
    Result Init(const Config& config, SaiHandle sai1, SaiHandle sai2);

    const Config& CetConfig() const;

	// Callback Function Typedefs
	// TODO: Add a 'void* data` member that can contain user data as well other details (like number of channels, etc.)
    typedef void (*AudioCallback)(float** in, float** out, size_t size);

    typedef void (*InterleavingAudioCallback)(float* in,
                                              float* out,
                                              size_t size);

    Result Start(AudioCallback callback);
    Result Start(InterleavingAudioCallback callback);
    Result Stop();

    class Impl;

  private:
    Impl* pimpl_;
};

} // namespace daisy

#endif 
