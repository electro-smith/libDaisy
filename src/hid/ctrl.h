#pragma once
#ifndef DSY_KNOB_H
#define DSY_KNOB_H /**< & */
#include <stdint.h>

#ifdef __cplusplus
namespace daisy
{
/**
 *  @brief Hardware Interface for control inputs \n 
 *  Primarily designed for ADC input controls such as \n 
 *  potentiometers, and control voltage. \n 
 *  @author Stephen Hensley
 *  @date November 2019
 *  @ingroup controls
 */
class AnalogControl
{
  public:
    /** Constructor */
    AnalogControl() {}
    /** destructor */
    ~AnalogControl() {}

    /** 
     * Initializes an AnalogControl.
     * 
     * @param adcptr a pointer to the raw ADC read value. This can be acquired with dsy_adc_get_rawptr() or dsy_adc_get_mux_rawptr()
     * @param sr the samplerate in Hz that the AnalogControl::Process function will be called at.
     * @param flip determines whether the input is flipped (i.e. 1.f - input) or not before being processed.1
     * @param invert determines whether the input is inverted (i.e. -1.f * input) or note before being processed.
     * @param slew_seconds is the slew time in seconds that it takes for the control to change to a new value.
     */
    void Init(uint16_t* adcptr,
              float     sr,
              bool      flip         = false,
              bool      invert       = false,
              float     slew_seconds = 0.002f);

    /** 
     * Initializes an AnalogControl to handle -5V to 5V inverted input.
     * 
     * All other aspects of the initialization are the same as those of
     * AnalogControl::Init.
     * 
     * @param *adcptr Pointer to analog digital converter
     * @param sr Audio engine sample rate
     */
    void InitBipolarCv(uint16_t* adcptr, float sr);

    /** 
     * Filters, and transforms a raw ADC read into a normalized range.
     * 
     * This method should be called at the rate specified when intiailizing the
     * control with AnalogControl::Init or AnalogControl::InitBipolarCv.   
     * 
     * If the control was initialized with AnalogControl::Init, this method
     * returns values in the range [0.0, 1.0], inclusive. 
     * 
     * If the conrol was initialized with AnalogControl::InitBipolarCv, this
     * method returns values in the range [-1.0, 1.0], inclusive.
     */
    float Process();

    /** 
     * Returns the value currently stored by the AnalogControl, without
     * reprocessing.
     */
    inline float Value() const { return val_; }

    /** Sets the coefficient of the one pole smoothing filter.
     * 
     *  @param val value to set coefficient to, must be in the range [0.0, 1.0], inclusive.
     */
    inline void SetCoeff(float val)
    {
        // using conditionals since clamp() is unavailable
        val = val > 1.f ? 1.f : val;
        val = val < 0.f ? 0.f : val;

        coeff_ = val;
    }

    /** Returns the raw unsigned 16-bit value from the ADC. */
    inline uint16_t GetRawValue() { return *raw_; }

    /** Returns a normalized float value representing the current ADC value. */
    inline float GetRawFloat() { return (float)(*raw_) / 65535.f; }

    /**
     * Sets a new sampling rate for an AnalogControl after the control has
     * already been initialized via AnalogControl::Init or
     * AnalogControl::InitBipolarCv. 
     *
     * @param sample_rate New update rate for the switch in hz
     */
    void SetSampleRate(float sample_rate);

  private:
    uint16_t* raw_;
    float     coeff_, samplerate_, val_;
    float     scale_, offset_;
    bool      flip_;
    bool      invert_;
    bool      is_bipolar_;
    float     slew_seconds_;
};
} // namespace daisy
#endif // __cplusplus

// C API:
typedef struct dsy_analog_ctrl {}dsy_analog_ctrl;

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
    /** Returns a new analog control. 
     * 
     * This function is a wrapper for AnalogControl::AnalogControl.
     */
    dsy_analog_ctrl* dsy_ctrl_new();

    /** 
     * Destroy a control, freeing associated memory. 
     *
     * This function is a wrapper for AnalogControl::~AnalogControl.
     */
    void dsy_ctrl_destroy(dsy_analog_ctrl* ctrl);

    /** 
     * Initializes an analog control. 
     * 
     * This function is a wrapper for AnalogControl::Init.
     */
    void dsy_ctrl_init(dsy_analog_ctrl* ctrl,
                       uint16_t*        adcptr,
                       float            sr,
                       bool             flip,
                       bool             invert,
                       float            slew_seconds);
    /** 
     * Initializes an analog control for inverted input.
     * 
     * This function is a wrapper for AnalogControl::InitBipolarCv.
     */
    void
    dsy_ctrl_init_bipolar_cv(dsy_analog_ctrl* ctrl, uint16_t* adcptr, float sr);

    /** Reads a value from an analog control's ADC and transforms it into a normalized range. 
     * 
     * This function is a wrapper for AnalogControl::Process. 
     */
    float dsy_ctrl_process(dsy_analog_ctrl* ctrl);

    /**
     * Returns the latest value from an analog control without reprocessing.
     * 
     * This function is a wrapper for AnalogControl::Value.
     */
    inline float dsy_ctrl_value(dsy_analog_ctrl* ctrl);

    /** Sets the coefficient for an analog control's one pole smoothing filter.  
     * 
     * This function is a wrapper for AnalogControl::SetCoeff.
     */
    inline void dsy_ctrl_set_coeff(dsy_analog_ctrl* ctrl, float val);

    /**
     * Returns the raw unsigned 16 bit value from an analog control's ADC.
     * 
     * This function is a wrapper for AnalogControl::GetRawValue. 
     */
    inline uint16_t dsy_ctrl_get_raw_value(dsy_analog_ctrl* ctrl);

    /**
     * Returns the raw value from an analog control's ADC as a float.
     * 
     * This function is a wrapper for AnalogControl::GetRawFloat.
     */
    inline float dsy_ctrl_get_raw_float(dsy_analog_ctrl* ctrl);

    /** Sets the sampling rate of an analog control. 
     * 
     * This function is a wrapper for AnalogControl::SetSampleRate.
     */
    void dsy_ctrl_set_sample_rate(dsy_analog_ctrl* ctrl, float sr);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DSY_KNOB_H