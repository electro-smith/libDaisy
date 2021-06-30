#include "MappedValue.h"
#include <cmath>
#include <cstring>

namespace daisy
{
// ==========================================================================

// ==========================================================================

MappedFloatValue::MappedFloatValue(float       min,
                                   float       max,
                                   float       defaultValue,
                                   Mapping     mapping,
                                   const char* unitStr,
                                   uint8_t     numDecimals,
                                   bool        forceSign)
: value_(defaultValue),
  min_(min),
  max_(max),
  default_(defaultValue),
  mapping_(mapping),
  unitStr_(unitStr),
  numDecimals_(numDecimals),
  forceSign_(forceSign)
{
}

void MappedFloatValue::Set(float newValue)
{
    value_ = std::max(min_, std::min(max_, newValue));
}

void MappedFloatValue::AppentToString(FixedCapStrBase<char>& string) const
{
    string.AppendFloat(value_, numDecimals_, false, forceSign_);
    string.Append(unitStr_);
}

void MappedFloatValue::ResetToDefault()
{
    value_ = default_;
}

float MappedFloatValue::GetAs0to1() const
{
    switch(mapping_)
    {
        case Mapping::lin: return (value_ - min_) / (max_ - min_);
        case Mapping::log:
        {
            const float a = 1.0f / (log10f(max_ / min_));
            return std::max(0.0f, std::min(1.0f, a * log10f(value_ / min_)));
        }
        case Mapping::pow2:
        {
            const float valueSq = (value_ - min_) / (max_ - min_);
            return std::max(0.0f, std::min(1.0f, sqrtf(valueSq)));
        }
        default: return 0.0f;
    }
}

void MappedFloatValue::SetFrom0to1(float normalizedValue0to1)
{
    float v;
    switch(mapping_)
    {
        case Mapping::lin:
            v = normalizedValue0to1 * (max_ - min_) + min_;
            break;
        case Mapping::log:
        {
            const float a = 1.0f / log10f(max_ / min_);
            v             = min_ * powf(10, normalizedValue0to1 / a);
        }
        break;
        case Mapping::pow2:
        {
            const float valueSq = normalizedValue0to1 * normalizedValue0to1;
            v                   = min_ + valueSq * (max_ - min_);
        }
        break;
        default: value_ = 0.0f; return;
    }
    value_ = std::max(min_, std::min(max_, v));
}

void MappedFloatValue::Step(int16_t numSteps, bool useCoarseStepSize)
{
    const auto mapped = GetAs0to1();
    const auto step
        = numSteps
          * (useCoarseStepSize ? coarseStepSize0to1_ : fineStepSize0to1_);
    SetFrom0to1(std::max(0.0f, std::min(mapped + step, 1.0f)));
}

// ==========================================================================

// ==========================================================================

MappedIntValue::MappedIntValue(int         min,
                               int         max,
                               int         defaultValue,
                               int         stepSizeFine,
                               int         stepSizeCoarse,
                               const char* unitStr,
                               bool        forceSign)
: value_(defaultValue),
  min_(min),
  max_(max),
  default_(defaultValue),
  stepSizeFine_(stepSizeFine),
  stepSizeCoarse_(stepSizeCoarse),
  unitStr_(unitStr),
  forceSign_(forceSign)
{
}

void MappedIntValue::Set(int newValue)
{
    value_ = std::max(min_, std::min(max_, newValue));
}

void MappedIntValue::AppentToString(FixedCapStrBase<char>& string) const
{
    string.AppendInt(value_, forceSign_);
    string.Append(unitStr_);
}

void MappedIntValue::ResetToDefault()
{
    value_ = default_;
}

float MappedIntValue::GetAs0to1() const
{
    return float(value_ - min_) / float(max_ - min_);
}

void MappedIntValue::SetFrom0to1(float normalizedValue0to1)
{
    const auto v = int(normalizedValue0to1 * (max_ - min_) + 0.5f) + min_;
    value_       = std::max(min_, std::min(max_, v));
}


void MappedIntValue::Step(int16_t numStepsUp, bool useCoarseStepSize)
{
    const auto stepsize = useCoarseStepSize ? stepSizeCoarse_ : stepSizeFine_;
    value_ = std::max(min_, std::min(max_, value_ + numStepsUp * stepsize));
}

// ==========================================================================

// ==========================================================================
MappedStringListValue::MappedStringListValue(const char** itemStrings,
                                             uint16_t     numItems,
                                             uint32_t     defaultIndex)
: index_(std::min(uint32_t(numItems - 1), defaultIndex)),
  itemStrings_(itemStrings),
  numItems_(numItems),
  defaultIndex_(std::min(uint32_t(numItems - 1), defaultIndex))
{
}

void MappedStringListValue::SetIndex(uint32_t index)
{
    index_ = std::min(uint32_t(numItems_ - 1), index);
}

void MappedStringListValue::AppentToString(FixedCapStrBase<char>& string) const
{
    string.Append(itemStrings_[index_]);
}

void MappedStringListValue::ResetToDefault()
{
    index_ = defaultIndex_;
}

float MappedStringListValue::GetAs0to1() const
{
    return float(index_) / float(numItems_ - 1);
}

void MappedStringListValue::SetFrom0to1(float normalizedValue0to1)
{
    index_ = std::max(
        0, std::min(int(normalizedValue0to1 * numItems_), numItems_ - 1));
}

void MappedStringListValue::Step(int16_t numStepsUp, bool useCoarseStepSize)
{
    if(numStepsUp > 0)
    {
        if(useCoarseStepSize)
            index_ = numItems_ - 1;
        else
            index_ = std::min(uint32_t(numItems_ - 1), index_ + numStepsUp);
    }
    else
    {
        if(useCoarseStepSize)
            index_ = 0;
        else
            index_ = uint32_t(std::max(0, int(index_) + numStepsUp));
    }
}

} // namespace daisy