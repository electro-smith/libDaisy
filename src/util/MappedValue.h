#pragma once
#include <stdint.h>
#include "FixedCapStr.h"

namespace daisy
{
/** @brief Abstract base class for a value that is mapped to a 0..1 normalized range.
 *  @author jelliesen
 *  @addtogroup utility
 */
class MappedValue
{
  public:
    virtual ~MappedValue() {}

    /** Generates a string representation and adds it to an existing string.
     * @param string    The string to add to
     */
    virtual void AppentToString(FixedCapStrBase<char>& string) const = 0;

    /** Resets the value to its default. */
    virtual void ResetToDefault() = 0;

    /** Returns the 0..1 normalized representation of the value, 
     *  e.g. to display a slider/knob on a UI. */
    virtual float GetAs0to1() const = 0;

    /** Sets the value so that `GetAs0to1()` returns `normalizedValue0to1`. */
    virtual void SetFrom0to1(float normalizedValue0to1) = 0;

    /** Steps the value up by whatever is appropriate. This function can be used
     *  to increment/decrement the value with buttons/encoders while making use 
     *  of the specific mapping. */
    virtual void Step(int16_t numStepsUp, bool useCoarseStepSize) = 0;
};

/** @brief A `MappedValue` that maps a float value using various mapping functions.
 *  @author jelliesen
 *  @addtogroup utility
 */
class MappedFloatValue : public MappedValue
{
  public:
    /** The availablke mapping functions */
    enum class Mapping
    {
        /** The value is mapped linearly between min and max. */
        lin,
        /** The value is mapped logarithmically. Note that the valid 
         *  values must be strictly larger than zero, so: min > 0, max > 0
         */
        log,
        /** The value is mapped with a square law */
        pow2
    };

    /** Creates a MappedFloatValue.
     * @param min           The lower end of the range of possible values
     * @param max           The upper end of the range of possible values
     * @param defaultValue  The default value
     * @param mapping       The `Mapping` to use. Note that for `Mapping::log`
     *                      `min`, `max` and ``defaultValue` must be > 0
     * @param unitStr       A string for the unit, e.g. "Hz"
     * @param numDecimals   Controls the number of decimals in `AppendToString()`
     * @param forceSign     Controls whether `AppendToString()` always prints the sign, 
     *                      even for positive numbers
    */
    MappedFloatValue(float       min,
                     float       max,
                     float       defaultValue,
                     Mapping     mapping     = Mapping::lin,
                     const char* unitStr     = "",
                     uint8_t     numDecimals = 1,
                     bool        forceSign   = false);

    ~MappedFloatValue() override {}

    /** Returns the current value. */
    float Get() const { return value_; }

    /** Returns a const pointer to the current value. */
    const float* GetPtr() const { return &value_; }

    /** Sets the value, clamping it to the valid range. */
    void Set(float newValue);

    /** Returns the current value. */
    operator float() const { return value_; }

    /** Sets the value, clamping it to the valid range. */
    MappedFloatValue& operator=(float val)
    {
        Set(val);
        return *this;
    }

    // inherited form MappedValue
    void AppentToString(FixedCapStrBase<char>& string) const override;

    // inherited form MappedValue
    void ResetToDefault() override;

    // inherited form MappedValue
    float GetAs0to1() const override;

    // inherited form MappedValue
    void SetFrom0to1(float normalizedValue0to1) override;

    /** Steps the 0..1 normalized representation of the value up or down
     *  in 1% or 5% steps.
     */
    void Step(int16_t numStepsUp, bool useCoarseStepSize) override;

  private:
    float                  value_;
    const float            min_;
    const float            max_;
    const float            default_;
    Mapping                mapping_;
    const char*            unitStr_;
    const uint8_t          numDecimals_;
    const bool             forceSign_;
    static constexpr float coarseStepSize0to1_ = 0.05f;
    static constexpr float fineStepSize0to1_   = 0.01f;
};

/** @brief A `MappedValue` that maps an int value linearly.
 *  @author jelliesen
 *  @addtogroup utility
 */
class MappedIntValue : public MappedValue
{
  public:
    /** Creates a MappedIntValue.
     * @param min             The lower end of the range of possible values
     * @param max             The upper end of the range of possible values
     * @param defaultValue    The default value
     * @param stepSizeFine    A fine step size to use in the `Step()` function
     * @param stepSizeCoarse  A coarse step size to use in the `Step()` function
     * @param unitStr         A string for the unit, e.g. "Hz"
     * @param forceSign       Controls whether `AppendToString()` always prints the sign, 
     *                        even for positive numbers
     */
    MappedIntValue(int         min,
                   int         max,
                   int         defaultValue,
                   int         stepSizeFine,
                   int         stepSizeCoarse,
                   const char* unitStr   = "",
                   bool        forceSign = false);

    ~MappedIntValue() override {}

    /** Returns the current value. */
    int Get() const { return value_; }

    /** Returns a const pointer to the current value. */
    const int* GetPtr() const { return &value_; }

    /** Sets the value, clamping it to the valid range. */
    void Set(int newValue);

    /** Returns the current value. */
    operator int() const { return value_; }

    /** Sets the value, clamping it to the valid range. */
    MappedIntValue& operator=(int val)
    {
        Set(val);
        return *this;
    }

    // inherited form MappedValue
    void AppentToString(FixedCapStrBase<char>& string) const override;

    // inherited form MappedValue
    void ResetToDefault() override;

    // inherited form MappedValue
    float GetAs0to1() const override;

    // inherited form MappedValue
    void SetFrom0to1(float normalizedValue0to1) override;

    /** Steps the value up or down using the step sizes specified in the constructor. */
    void Step(int16_t numStepsUp, bool useCoarseStepSize) override;

  private:
    int         value_;
    const int   min_;
    const int   max_;
    const int   default_;
    const int   stepSizeFine_;
    const int   stepSizeCoarse_;
    const char* unitStr_;
    const bool  forceSign_;
};
/** @brief A `MappedValue` that maps an list of strings linearly.
 *  @author jelliesen
 *  @addtogroup utility
 */
class MappedStringListValue : public MappedValue
{
  public:
    /** Creates a MappedStringListValue.
     * @param itemStrings     An Array of strings, one for each possible item.
     * @param numItems        The number of possible items
     * @param defaultIndex    The default index
     */
    MappedStringListValue(const char** itemStrings,
                          uint16_t     numItems,
                          uint32_t     defaultIndex);

    ~MappedStringListValue() override {}

    /** Returns the current item index. */
    int GetIndex() const { return index_; }

    /** Returns the current item string. */
    const char* GetString() const { return itemStrings_[index_]; }

    /** Returns a pointer to the current item index. */
    const uint32_t* GetIndexPtr() const { return &index_; }

    /** Sets the current item index, clamping it to a valid item index. */
    void SetIndex(uint32_t index);

    /** Returns the current item index. */
    operator int() const { return index_; }

    /** Returns the current item string. */
    operator const char*() const { return itemStrings_[index_]; }

    /** Sets the current item index, clamping it to a valid item index. */
    MappedStringListValue& operator=(int index)
    {
        SetIndex(index);
        return *this;
    }

    // inherited form MappedValue
    void AppentToString(FixedCapStrBase<char>& string) const override;

    // inherited form MappedValue
    void ResetToDefault() override;

    // inherited form MappedValue
    float GetAs0to1() const override;

    // inherited form MappedValue
    void SetFrom0to1(float normalizedValue0to1) override;

    /** Steps through the items up or down. If the coarse step size is used, the
     *  value will jump to the first or last item. */
    void Step(int16_t numStepsUp, bool useCoarseStepSize) override;

  private:
    uint32_t     index_;
    const char** itemStrings_;
    const int    numItems_;
    uint32_t     defaultIndex_;
};


} // namespace daisy