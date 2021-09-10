#pragma once

#include "daisy.h"

#define A1 daisy::DaisyPatchSM::PinBank::A, 1
#define A2 daisy::DaisyPatchSM::PinBank::A, 2
#define A3 daisy::DaisyPatchSM::PinBank::A, 3
#define A4 daisy::DaisyPatchSM::PinBank::A, 4
#define A5 daisy::DaisyPatchSM::PinBank::A, 5
#define A6 daisy::DaisyPatchSM::PinBank::A, 6
#define A7 daisy::DaisyPatchSM::PinBank::A, 7
#define A8 daisy::DaisyPatchSM::PinBank::A, 8
#define A9 daisy::DaisyPatchSM::PinBank::A, 9
#define A10 daisy::DaisyPatchSM::PinBank::A, 10
#define B1 daisy::DaisyPatchSM::PinBank::B, 1
#define B2 daisy::DaisyPatchSM::PinBank::B, 2
#define B3 daisy::DaisyPatchSM::PinBank::B, 3
#define B4 daisy::DaisyPatchSM::PinBank::B, 4
#define B5 daisy::DaisyPatchSM::PinBank::B, 5
#define B6 daisy::DaisyPatchSM::PinBank::B, 6
#define B7 daisy::DaisyPatchSM::PinBank::B, 7
#define B8 daisy::DaisyPatchSM::PinBank::B, 8
#define B9 daisy::DaisyPatchSM::PinBank::B, 9
#define B10 daisy::DaisyPatchSM::PinBank::B, 10
#define C1 daisy::DaisyPatchSM::PinBank::C, 1
#define C2 daisy::DaisyPatchSM::PinBank::C, 2
#define C3 daisy::DaisyPatchSM::PinBank::C, 3
#define C4 daisy::DaisyPatchSM::PinBank::C, 4
#define C5 daisy::DaisyPatchSM::PinBank::C, 5
#define C6 daisy::DaisyPatchSM::PinBank::C, 6
#define C7 daisy::DaisyPatchSM::PinBank::C, 7
#define C8 daisy::DaisyPatchSM::PinBank::C, 8
#define C9 daisy::DaisyPatchSM::PinBank::C, 9
#define C10 daisy::DaisyPatchSM::PinBank::C, 10
#define D1 daisy::DaisyPatchSM::PinBank::D, 1
#define D2 daisy::DaisyPatchSM::PinBank::D, 2
#define D3 daisy::DaisyPatchSM::PinBank::D, 3
#define D4 daisy::DaisyPatchSM::PinBank::D, 4
#define D5 daisy::DaisyPatchSM::PinBank::D, 5
#define D6 daisy::DaisyPatchSM::PinBank::D, 6
#define D7 daisy::DaisyPatchSM::PinBank::D, 7
#define D8 daisy::DaisyPatchSM::PinBank::D, 8
#define D9 daisy::DaisyPatchSM::PinBank::D, 9
#define D10 daisy::DaisyPatchSM::PinBank::D, 10


namespace daisy
{
/** @brief Board support file for DaisyPatchSM hardware
 *  @author shensley
 *  @ingroup boards
 * 
 *  Daisy Patch SM is a complete Eurorack module DSP engine.
 *  Based on the Daisy Seed, with circuits added for 
 *  interfacing directly with eurorack modular synthesizers.
 */
class DaisyPatchSM
{
  public:
    /** Helper for mapping pins, and accessing them using the `GetPin` function */
    enum class PinBank
    {
        A,
        B,
        C,
        D
    };

    /** Accessors for the Analog Controls. 
     *  These cover the 8x Bipolar CV inputs
     *  as well as the 4x 0-3V3 ADC inputs on
     *  the hardware 
     * 
     *  When reading a value with GetAdcValue()
     * 
     *  You can use either (assuming obj. is named "patch"):
     *  
     *  patch.GetAdcValue(patch.CV_1);
     *  or
     *  patch.GetAdcValue(DaisyPatchSM::CV_1);
     */
    enum
    {
        CV_1 = 0x00,
        CV_2,
        CV_3,
        CV_4,
        CV_5,
        CV_6,
        CV_7,
        CV_8,
        ADC_9,
        ADC_10,
        ADC_11,
        ADC_12,
        ADC_LAST,
    };

    /** Enum for addressing the CV Outputs via the WriteCvOut function. */
    enum
    {
        CV_OUT_BOTH = 0x00,
        CV_OUT_1,
        CV_OUT_2,
    };

    DaisyPatchSM() {}
    ~DaisyPatchSM() {}

    /** Initializes the memories, and core peripherals for the Daisy Patch SM */
    void Init();

    /** Starts a non-interleaving audio callback */
    void StartAudio(AudioHandle::AudioCallback cb);

    /** Starts an interleaving audio callback */
    void StartAudio(AudioHandle::InterleavingAudioCallback cb);

    /** Changes the callback that is executing.
     *  This may cause clicks if done while audio is processing.
     */
    void ChangeAudioCallback(AudioHandle::AudioCallback cb);

    /** Changes the callback that is executing.
     *  This may cause clicks if done while audio is processing.
     */
    void ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb);

    /** Stops the transmission of audio. */
    void StopAudio();

    /** Sets the number of samples processed in an audio callback. 
     *  This will only take effect on the next invocation of `StartAudio`
     */
    void SetAudioBlockSize(size_t size);

    /** Sets the samplerate for the audio engine 
     *  This will set it to the closest valid samplerate. Options being:
     *  8kHz, 16kHz, 32kHz, 48kHz, and 96kHz
     */
    void SetAudioSampleRate(float sr);

    /** Returns the number of samples processed in an audio callback */
    size_t AudioBlockSize();

    /** Returns the audio engine's samplerate in Hz */
    float AudioSampleRate();

    /** Returns the rate at which the audio callback will be called in Hz */
    float AudioCallbackRate();

    /** Starts the Control ADCs 
     * 
     *  This is started automatically when Init() is called.
     */
    void StartAdc();

    /** Stops the Control ADCs */
    void StopAdc();

    /** Reads and filters all of the analog control inputs */
    void ProcessAnalogControls();

    /** Reads and debounces any of the digital control inputs 
     *  This does nothing on this board at this time.
    */
    void ProcessDigitalControls();

    /** Does both of the above */
    void ProcessAllControls()
    {
        ProcessAnalogControls();
        ProcessDigitalControls();
    }

    /** Returns the current value for one of the ADCs */
    float GetAdcValue(int idx);

    /** Returns the STM32 port/pin combo for the desired pin (or an invalid pin for HW only pins)
     *
     *  Macros at top of file can be used in place of separate arguments (i.e. GetPin(A4), etc.)
     * 
     *  \param bank should be one of the PinBank options above
     *  \param idx pin number between 1 and 10 for each of the pins on each header.
     */
    dsy_gpio_pin GetPin(PinBank bank, int idx);

    /** Starts the DAC for the CV Outputs 
     * 
     *  By default this starts by running the 
     *  internal callback at 48kHz, which will 
     *  update the values based on the SetCvOut 
     *  function.
     * 
     *  This is started automatically when Init() is called.
     */
    void StartDac(DacHandle::DacCallback callback = nullptr);

    /** Stop the DAC from updating. 
     *  This will suspend the CV Outputs from changing 
     */
    void StopDac();

    /** Sets specified DAC channel to the target voltage. 
     *  This may not be 100% accurate without calibration. 
     *  
     *  \todo Add Calibration to CV Outputs
     * 
     *  \param channel desired channel to update. 0 is both, otherwise 1 or 2 are valid.
     *  \param volage value in Volts that you'd like to write to the DAC
     */
    void WriteCvOut(const int channel, float voltage);


    /** Here are some wrappers around libDaisy Static functions 
     *  to provide simpler syntax to those who prefer it. */

    /** Delays for a specified number of milliseconds */
    inline void Delay(uint32_t milliseconds) { System::Delay(milliseconds); }

    /** Gets a random 32-bit value */
    inline uint32_t GetRandomValue() { return Random::GetValue(); }

    /** Gets a random floating point value between the specified minimum, and maxmimum */
    inline float GetRandomFloat(float min = 0.f, float max = 1.f)
    {
        return Random::GetFloat(min, max);
    }

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

    /** Tests entirety of SDRAM for validity 
     *  This will wipe contents of SDRAM when testing. 
     * 
     *  If using the SDRAM for the default bss, or heap, and using constructors as initializers
     *  do not call this function.
     *  Otherwise, it could overwrite changes performed by constructors.
     * 
     *  \retval returns true if SDRAM is okay, otherwise false
    */
    bool ValidateSDRAM();

    /** Direct Access Structs/Classes */
    System           system;
    dsy_sdram_handle sdram;
    QSPIHandle       qspi;
    AudioHandle      audio;
    AdcHandle        adc;
    UsbHandle        usb;
    Pcm3060          codec;
    DacHandle        dac;

    /** Dedicated Function Pins */
    AnalogControl controls[ADC_LAST];
    GateIn        gate_in_1, gate_in_2;
    dsy_gpio      gate_out_1, gate_out_2;

    class Impl;


  private:
    using Log = Logger<LOGGER_INTERNAL>;

    float callback_rate_;

    /** Background callback for updating the DACs. */
    Impl* pimpl_;
};

} // namespace daisy
