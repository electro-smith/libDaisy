// This is a Board Specific File
// I don't think it actually belongs in the library.
// Any new piece of hardware can just have their own board file.
// This will allow minor pin changes, etc. not to require changing the library in a million places.

#pragma once
#ifndef DSY_SEED_H
#define DSY_SEED_H

#include "daisy.h"

// Uncomment this out if you're still using the rev2 hardware
//#define SEED_REV2

namespace daisy
{
class DaisySeed
{
  public:
    DaisySeed() {}
    ~DaisySeed() {}

    // ## Initialization and Startup
    // ### Configure
    // configures the settings for all internal peripherals,
    // but does not initialize them.
    //
    // This allows for modification of the configuration
    // handles prior to initialization.
    //
    // Defaults listed below:
    //
    // TODO: Add defaults
	// ~~~~
    void Configure();
	// ~~~~

    // ### Init
    // Initializes the Daisy Seed and the following peripherals:
    // SDRAM, QSPI, 24-bit 48kHz Audio via AK4556, Internal USB,
    // as well as the built-in LED and Testpoint.
    //
    // ADCs, DACs, and other special peripherals (such as I2C, SPI, etc.)
    // can be initialized using their specific initializers within libdaisy
    // for a specific application.
	// ~~~~
    void Init();
	// ~~~~

    // ## Helpers and More
    // ### GetPin
    // Returns the gpio_pin corresponding to the index 0-31.
    // For the given GPIO on the Daisy Seed (labeled 1-32 in docs).
	// ~~~~
    dsy_gpio_pin GetPin(uint8_t pin_idx);
	// ~~~~

    // ### StartAudio
    // Begins the audio for the seeds builtin audio.
    // the specified callback will get called whenever
    // new data is ready to be prepared.
	// ~~~~
    void StartAudio(dsy_audio_callback cb);
	// ~~~~

    // ### SetLed
    // Sets the state of the built in LED
	// ~~~~
    void SetLed(bool state);
	// ~~~~

    // ### SetTestPoint
    // Sets the state of the test point near pin 10
	// ~~~~
    void SetTestPoint(bool state);
	// ~~~~
    
    // ### AudioSampleRate
    // Returns the audio sample rate in Hz as a floating point number.
    // ~~~~
    float  AudioSampleRate();
    // ~~~~
    
    // ### SetAudioBlockSize
    // Sets the number of samples processed per channel by the audio callback.
    // ~~~~
    void SetAudioBlockSize(size_t blocksize);
    // ~~~~

    // ## Public Members
    // While the library is still in heavy development, most of the
    // configuration handles will remain public.
	// ~~~~
    dsy_sdram_handle sdram_handle;
    dsy_qspi_handle  qspi_handle;
    dsy_audio_handle audio_handle;
    dsy_sai_handle   sai_handle;
    dsy_i2c_handle   i2c1_handle, i2c2_handle;
    AdcHandle adc;
    dsy_dac_handle   dac_handle;
    UsbHandle        usb_handle;
	// ~~~~

  private:
    void     ConfigureSdram();
    void     ConfigureQspi();
    void     ConfigureAudio();
    void     ConfigureAdc();
    void     ConfigureDac();
    void     ConfigureI2c();
    dsy_gpio led_, testpoint_;
};

} // namespace daisy

#endif
