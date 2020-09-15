#include "hid/audio.h"

namespace daisy
{

// ================================================================
// Static Globals
// ================================================================

// Ordinarily the user would be required to provide their own buffer, but
// in the interest in encourage newcomers, and this also being an audio-centric platform
// these buffers will always be present, and usable.
//
// Static Global Buffers
// 8kB in SRAM1, non-cached memory
// 1k samples in, 1k samples out, 4 bytes per sample.
static const size_t kAudioMaxBufferSize = 1024;
static int32_t DMA_BUFFER_MEM_SECTION dsy_audio_rx_buffer[kAudioMaxBufferSize];
static int32_t DMA_BUFFER_MEM_SECTION dsy_audio_tx_buffer[kAudioMaxBufferSize];

// ================================================================
// Private Implementation Definition
// ================================================================
class AudioHandle::Impl
{
  public:
    // Interface
    AudioHandle::Result Init(const AudioHandle::Config config, SaiHandle sai);
    AudioHandle::Result
                        Init(const AudioHandle::Config config, SaiHandle sai1, SaiHandle sai2);
    AudioHandle::Result Start(AudioHandle::AudioCallback callback);
    AudioHandle::Result Start(AudioHandle::InterleavingAudioCallback callback);
    AudioHandle::Result Stop();

    // Internal Callback
    static void InternalCallback(int32_t* in, int32_t* out, size_t size);

    void *callback_, *interleaved_callback_;

    // Data
    AudioHandle::Config config_;
    SaiHandle           sai1_, sai2_;
    int32_t*            buff_rx_;
    int32_t*            buff_tx_;
};

// ================================================================
// Static Reference for Object
// ================================================================

static AudioHandle::Impl audio_handle;

// ================================================================
// Private Implementation
// ================================================================

AudioHandle::Result AudioHandle::Impl::Init(const AudioHandle::Config config,
                                            SaiHandle                 sai)
{
    config_ = config;
    if(sai.IsInitialized())
        sai1_ = sai;
    else
        return Result::ERR;
    buff_rx_ = dsy_audio_rx_buffer;
    buff_tx_ = dsy_audio_tx_buffer;
    return Result::OK;
}

AudioHandle::Result AudioHandle::Impl::Init(const AudioHandle::Config config,
                                            SaiHandle                 sai1,
                                            SaiHandle                 sai2)
{
    this->Init(config, sai1);
    sai2_ = sai2;
    // How do we want to handle the rx/tx buffs for the second peripheral of audio..?
    return Result::OK;
}


AudioHandle::Result
AudioHandle::Impl::Start(AudioHandle::AudioCallback callback)
{
    // Get instance of object
    sai1_.StartDma(buff_rx_,
                   buff_tx_,
                   config_.blocksize * 2 * 2,
                   audio_handle.InternalCallback);
    callback_             = (void*)callback;
    interleaved_callback_ = nullptr;
    return Result::OK;
}
AudioHandle::Result
AudioHandle::Impl::Start(AudioHandle::InterleavingAudioCallback callback)
{
    // Get instance of object
    sai1_.StartDma(buff_rx_,
                   buff_tx_,
                   config_.blocksize * 2 * 2,
                   audio_handle.InternalCallback);
    interleaved_callback_ = (void*)callback;
    callback_             = nullptr;
    return Result::OK;
}
AudioHandle::Result AudioHandle::Impl::Stop()
{
    sai1_.StopDma();
    return Result::OK;
}

// This turned into a very large function due to the bit-depth conversions..
// I didn't want to do a conditional conversion in a single loop because it seemed
// far less efficient, so this is where I landed.
//
// Using function pointers for the x2f and f2x functions would have been ideal, but
// wasn't possible due to the different parameter/return types for each function.
void AudioHandle::Impl::InternalCallback(int32_t* in, int32_t* out, size_t size)
{
    // Convert from sai format to float, and call user callback
    SaiHandle::Config::BitDepth bd;
    bd = audio_handle.sai1_.GetConfig().bit_depth;
    // Handle Interleaved / Non Interleaved separate
    if(audio_handle.interleaved_callback_)
    {
        InterleavingAudioCallback cb
            = (InterleavingAudioCallback)audio_handle.interleaved_callback_;
        float fin[size];
        float fout[size];
        // There _must_ be a more elegant way to do this....
        // Convert from int to float
        switch(bd)
        {
            case SaiHandle::Config::BitDepth::SAI_16BIT:
                for(size_t i = 0; i < size; i += 2)
                {
                    fin[i]     = s162f(in[i]);
                    fin[i + 1] = s162f(in[i + 1]);
                }
                break;
            case SaiHandle::Config::BitDepth::SAI_24BIT:
                for(size_t i = 0; i < size; i += 2)
                {
                    fin[i]     = s242f(in[i]);
                    fin[i + 1] = s242f(in[i + 1]);
                }
                break;
            case SaiHandle::Config::BitDepth::SAI_32BIT:
                for(size_t i = 0; i < size; i += 2)
                {
                    fin[i]     = s322f(in[i]);
                    fin[i + 1] = s322f(in[i + 1]);
                }
                break;
            default: break;
        }
        cb(fin, fout, size);
        switch(bd)
        {
            case SaiHandle::Config::BitDepth::SAI_16BIT:
                for(size_t i = 0; i < size; i += 2)
                {
                    out[i]     = f2s16(fout[i]);
                    out[i + 1] = f2s16(fout[i + 1]);
                }
                break;
            case SaiHandle::Config::BitDepth::SAI_24BIT:
                for(size_t i = 0; i < size; i += 2)
                {
                    out[i]     = f2s24(fout[i]);
                    out[i + 1] = f2s24(fout[i + 1]);
                }
                break;
            case SaiHandle::Config::BitDepth::SAI_32BIT:
                for(size_t i = 0; i < size; i += 2)
                {
                    out[i]     = f2s32(fout[i]);
                    out[i + 1] = f2s32(fout[i + 1]);
                }
                break;
            default: break;
        }
    }
    else if(audio_handle.callback_)
    {
        AudioCallback cb = (AudioCallback)audio_handle.callback_;
        float         finbuff[size], foutbuff[size];
        float*        fin[2];
        float*        fout[2];
        fin[0]  = finbuff;
        fin[1]  = finbuff + (size / 2);
        fout[0] = foutbuff;
        fout[1] = foutbuff + (size / 2);
        // Deinterleave and scale
        switch(bd)
        {
            case SaiHandle::Config::BitDepth::SAI_16BIT:
                for(size_t i = 0; i < size; i += 2)
                {
                    fin[0][i / 2] = s162f(in[i]);
                    fin[1][i / 2] = s162f(in[i + 1]);
                }
                break;
            case SaiHandle::Config::BitDepth::SAI_24BIT:
                for(size_t i = 0; i < size; i += 2)
                {
                    fin[0][i / 2] = s242f(in[i]);
                    fin[1][i / 2] = s242f(in[i + 1]);
                }
                break;
            case SaiHandle::Config::BitDepth::SAI_32BIT:
                for(size_t i = 0; i < size; i += 2)
                {
                    fin[0][i / 2] = s322f(in[i]);
                    fin[1][i / 2] = s322f(in[i + 1]);
                }
                break;
            default: break;
        }
        cb((float**)fin, (float**)fout, size / 2);
        // Reinterleave and scale
        switch(bd)
        {
            case SaiHandle::Config::BitDepth::SAI_16BIT:
                for(size_t i = 0; i < size; i += 2)
                {
                    out[i]     = f2s16(fout[0][i / 2]);
                    out[i + 1] = f2s24(fout[1][i / 2]);
                }
                break;
            case SaiHandle::Config::BitDepth::SAI_24BIT:
                for(size_t i = 0; i < size; i += 2)
                {
                    out[i]     = f2s24(fout[0][i / 2]);
                    out[i + 1] = f2s24(fout[1][i / 2]);
                }
                break;
            case SaiHandle::Config::BitDepth::SAI_32BIT:
                for(size_t i = 0; i < size; i += 2)
                {
                    out[i]     = f2s32(fout[0][i / 2]);
                    out[i + 1] = f2s32(fout[1][i / 2]);
                }
                break;
            default: break;
        }
    }
}

// ================================================================
// SaiHandle -> SaiHandle::Pimpl
// ================================================================

AudioHandle::Result AudioHandle::Init(const Config& config, SaiHandle sai)
{
    // Figure out proper pattern for singleton behavior here.
    pimpl_ = &audio_handle;
    return pimpl_->Init(config, sai);
}

AudioHandle::Result
AudioHandle::Init(const Config& config, SaiHandle sai1, SaiHandle sai2)
{
    // Figure out proper pattern for singleton behavior here.
    pimpl_ = &audio_handle;
    return pimpl_->Init(config, sai1, sai2);
}
AudioHandle::Result AudioHandle::Start(AudioCallback callback)
{
    return pimpl_->Start(callback);
}
AudioHandle::Result AudioHandle::Start(InterleavingAudioCallback callback)
{
    return pimpl_->Start(callback);
}
AudioHandle::Result AudioHandle::Stop()
{
    return pimpl_->Stop();
}

} // namespace daisy
