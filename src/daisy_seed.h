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
    All basic peripheral configuration/initialization is setup here. \n

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
    void Init(bool boost = false);

    /** 
    Deinitializes all peripherals automatically handled by `Init`.
    */
    void DeInit();

    /** 
    Wait some ms before going on.
    \param del Delay time in ms.
    */
    void DelayMs(size_t del);

    /** 
    Returns the gpio_pin corresponding to the index 0-31.
    For the given GPIO on the Daisy Seed (labeled 1-32 in docs).
    */
    static dsy_gpio_pin GetPin(uint8_t pin_idx);

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

    /** Print formatted debug log message
     */
    template <typename... VA>
    static void Print(const char* format, VA... va)
    {
        Log::Print(format, va...);
    }

    /** Print formatted debug log message with automatic line termination
    */
    template <typename... VA>
    static void PrintLine(const char* format, VA... va)
    {
        Log::PrintLine(format, va...);
    }

    /** Start the logging session. Optionally wait for terminal connection before proceeding.
    */
    static void StartLog(bool wait_for_pc = false)
    {
        Log::StartLog(wait_for_pc);
    }


    // While the library is still in heavy development, most of the
    // configuration handles will remain public.
    QSPIHandle         qspi;
    QSPIHandle::Config qspi_config;
    SdramHandle        sdram_handle; /**< & */
    AudioHandle        audio_handle; /**< & */
    AdcHandle          adc;          /**< & */
    DacHandle          dac;
    UsbHandle          usb_handle; /**< & */
    dsy_gpio           led, testpoint;
    System             system;

  private:
    /** Local shorthand for debug log destination
    */
    using Log = Logger<LOGGER_INTERNAL>;

    /** Internal indices for DaisySeed-equivalent devices 
     *  This shouldn't have any effect on user-facing code,
     *  and only needs to be checked to properly initialize
     *  the onboard-circuits.
    */
    enum class BoardVersion
    {
        /** Daisy Seed Rev4
         *  This is the original Daisy Seed */
        DAISY_SEED,
        /** Daisy Seed 1.1 (aka Daisy Seed Rev5)
         *  This is a pin-compatible version of the Daisy Seed
         *  that uses the WM8731 codec instead of the AK4430 */
        DAISY_SEED_1_1,
    };
    BoardVersion CheckBoardVersion();

    void ConfigureQspi();
    void ConfigureAudio();
    void ConfigureAdc();
    void ConfigureDac();
    //void     ConfigureI2c();
    float callback_rate_;
};

} // namespace daisy

#endif
