#ifndef DSY_AUDIO_H
#define DSY_AUDIO_H /**< & */
#include <stddef.h>
#include <stdint.h>
#include "per/sai.h"
//#include "per/i2c.h"
#ifdef __cplusplus
extern "C"
{
#endif


/** Defines for generic maximums
    While 'Audio Channels Max' is set to 2, this is per-SAI 
    4x4 Audio I/O is possible using the dsy_audio_mc_callback
    Hard-coded samplerate is calculated from original clock tree.  
    The new clock tree has less than 0.01% error for all supported samplerates
*/
/** Max block size */
#define DSY_AUDIO_BLOCK_SIZE_MAX 128
/** Max number of audio channels */
#define DSY_AUDIO_CHANNELS_MAX 2

#ifndef DSY_AUDIO_SAMPLE_RATE
#define DSY_AUDIO_SAMPLE_RATE 48014.0f /**< Default sample rate */
#endif

    /** @ingroup audio 
    @{ 
    */

    /*
      brief Audio Driver \n 
    Configures Audio Device and provides callback for signal processing. \n 
    Many of the hard-coded values here will change (increase), and/or \n 
    be replaced by configurable options 
    */

    /** Internally, there are two separate 'audio blocks' that can
      be configured together or separately
  */
    enum
    {
        DSY_AUDIO_INTERNAL, /**< & */
        DSY_AUDIO_EXTERNAL, /**< & */
        DSY_AUDIO_LAST,     /**< & */
    };


    /** Simple config struct that holds peripheral drivers. */
    typedef struct
    {
        size_t          block_size; /**< & */
        dsy_sai_handle* sai;        /**< & */
        //dsy_i2c_handle* dev0_i2c;   /**< & */
        //dsy_i2c_handle* dev1_i2c;   /**< & */
    } dsy_audio_handle;

    /* typical example:
       void AudioCallback(float *in, float *out, size_t size)
       {
           for (size_t i = 0; i < size; i+=2)
       {
           out[i] = in[i]; // Left
           out[i+1] = in[i+1]; // Right
       }
       
       }
    */

    /** These are user-defineable callbacks that are called when 
    audio data is ready to be received/transmitted.
    Function to define for using a single Stereo device for I/O
    audio is packed as: { LEFT | RIGHT | LEFT | RIGHT }
    */
    typedef void (*dsy_audio_callback)(float*, float*, size_t);

    /* typical example:
    void AudioCallback(float **in, float **out, size_t size)
    {
    
        for (size_t i = 0; i < size; i++)
    {
            for (size_t chn = 0; chn < num_channels; chn++)
        {
            out[chn] = in[chn];
        }
    }
       
    }
    */

    /** Defaults to 4 channels, and is fixed for now. \n
    (still works for stereo, but will still fill buffers) \n
    audio is packed as: \n
    { LEFT | LEFT + 1 | . . . | LEFT + SIZE | RIGHT | RIGHT + 1 | . . . | RIGHT + SIZE  } \n
    */
    typedef void (*dsy_audio_mc_callback)(float**, float**, size_t);

    /** Initializes the Audio Engine using configurations \n 
    set to the sai_handle \n
    i2c_handles can be set to NULL if not needed. \n
     */
    void dsy_audio_init(dsy_audio_handle* handle);

    /** Sets the user defined, interleaving callback to be called when audio data is ready. \n
    intext is a specifier for DSY_AUDIO_INT/EXT (which audio peripheral to use). \n
    When using this, each 'audio block' can have completely independent callbacks. \n
    */
    void dsy_audio_set_callback(uint8_t intext, dsy_audio_callback cb);

    /** Sets the user defined, non-interleaving callback to be called when audio data is ready. \n
    This will always use both DSY_AUDIO_INT and DSY_AUDIO_EXT blocks together. \n
    To ensure clean audio you'll want to make sure the two SAIs are set to the same samplerate \n
    */
    void dsy_audio_set_mc_callback(dsy_audio_mc_callback cb);

    /** Sets the number of samples (per-channel) to be handled in a single audio frame.
     */
    void dsy_audio_set_blocksize(uint8_t intext, size_t blocksize);

    /** Starts Audio Engine, callbacks will begin getting called. \n
    When using with dsy_audio_mc_callback (for 4 channels), 
    this function should be called for both audio blocks
    */
    void dsy_audio_start(uint8_t intext);

    /** Stops transmitting/receiving audio on the specified audio block.
     */
    void dsy_audio_stop(uint8_t intext);

    //Only minimally tested with WM8731 codec.**
    /** If the device supports hardware bypass, enter that mode. */
    void dsy_audio_enter_bypass(uint8_t intext);


    // Only minimally tested with WM8731 codec.
    /** If the device supports hardware bypass, exit that mode. */
    void dsy_audio_exit_bypass(uint8_t intext);

    /** A few useful stereo-interleaved callbacks \n
    Passes the input to the output
    */
    void dsy_audio_passthru(float* in, float* out, size_t size);

    /** sets outputs to 0 without stopping the Audio Engine */
    void dsy_audio_silence(float* in, float* out, size_t size);

#ifdef __cplusplus
}
#endif
#endif
/** @} */
