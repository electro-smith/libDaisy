#pragma once
#ifndef DSY_SEED_H
#define DSY_SEED_H

#include "daisy.h"

//Uncomment this out if you're still using the rev2 hardware
//#define SEED_REV2

namespace daisy
{
/**
   @brief This is the higher-level interface for the Daisy board. \n 
    All basic peripheral configuration/initialization is setup here.
   @ingroup boards
*/
class DaisySeed
{
  public:
    DaisySeed() {}
    ~DaisySeed() {}

    /** 
    Configures the settings for all internal peripherals,
    but does not initialize them.
    This allows for modification of the configuration
    handles prior to initialization.
    */

    //Defaults listed below:
    //TODO: Add defaults

    /** & */
    void Configure();

    /** 
    Initializes the Daisy Seed and the following peripherals:
    SDRAM, QSPI, 24-bit 48kHz Audio via AK4556, Internal USB,
    as well as the built-in LED and Testpoint.

    ADCs, DACs, and other special peripherals (such as I2C, SPI, etc.)
    can be initialized using their specific initializers within libdaisy
    for a specific application.
    */
    void Init();

    /** 
    Returns the gpio_pin corresponding to the index 0-31.
    For the given GPIO on the Daisy Seed (labeled 1-32 in docs).
    */
    dsy_gpio_pin GetPin(uint8_t pin_idx);

    /** Begins the audio for the seeds builtin audio.
    the specified callback will get called whenever
    new data is ready to be prepared.
    */
    void StartAudio(AudioHandle::InterleavingAudioCallback cb);

    /** Begins the audio for the seeds builtin audio.
    the specified callback will get called whenever
    new data is ready to be prepared.
    This will use the newer non-interleaved callback.
    */
    void StartAudio(AudioHandle::AudioCallback cb);

    /** Changes to a new interleaved callback
     */
    void ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb);

    /** Changes to a new multichannel callback
     */
    void ChangeAudioCallback(AudioHandle::AudioCallback cb);

    /** Stops the audio if it is running. */
    void StopAudio();

    /** Updates the Audio Sample Rate, and reinitializes.
     ** Audio must be stopped for this to work.
     */
    void SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate);

    /** Returns the audio sample rate in Hz as a floating point number.
     */
    float AudioSampleRate();

    /** Sets the number of samples processed per channel by the audio callback.
     */
    void SetAudioBlockSize(size_t blocksize);

    /** Returns the number of samples per channel in a block of audio. */
    size_t AudioBlockSize();

    /** Returns the rate in Hz that the Audio callback is called */
    float AudioCallbackRate() const;

    /** Sets the state of the built in LED
     */
    void SetLed(bool state);

    /** Sets the state of the test point near pin 10
     */
    void SetTestPoint(bool state);

    // While the library is still in heavy development, most of the
    // configuration handles will remain public.
    dsy_sdram_handle sdram_handle; /**< & */
    dsy_qspi_handle  qspi_handle;  /**< & */
    AudioHandle      audio_handle; /**< & */
    AdcHandle        adc;          /**< & */
    dsy_dac_handle   dac_handle;   /**< & */
    UsbHandle        usb_handle;   /**< & */

  private:
    void ConfigureSdram();
    void ConfigureQspi();
    void ConfigureAudio();
    void ConfigureAdc();
    void ConfigureDac();
    //void     ConfigureI2c();
    dsy_gpio led_, testpoint_;
    float    callback_rate_;
};

} // namespace daisy

#endif
