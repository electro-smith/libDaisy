#include "rng.h"
#include "util/hal_map.h"
#include "sys/system.h"

#define RNG_TIMEOUT 100

namespace daisy
{

void RandomNumberGenerator::Init()
{
    /** NON-HAL except defines/macros */
    __HAL_RCC_RNG_CLK_ENABLE();
    MODIFY_REG(RNG->CR, RNG_CR_CED, RNG_CED_ENABLE);
    RNG->CR |= RNG_CR_RNGEN;
}

void RandomNumberGenerator::DeInit()
{
    __HAL_RCC_RNG_CLK_DISABLE();
}

uint32_t RandomNumberGenerator::GetValue()
{
    /** HAL code */
    // HAL_RNG_GenerateRandomNumber()
    uint32_t start;
    start = System::GetNow();

    while(!IsReady())
    {
        if(System::GetNow() - start > RNG_TIMEOUT)
        {
            /** Soft failure on error for now */
            return 0;
        }
    }
    uint32_t t = RNG->DR;
    return t;
}

bool RandomNumberGenerator::IsReady()
{
    return ((RNG->SR & RNG_FLAG_DRDY) == RNG_FLAG_DRDY) == SET;
}

} // namespace daisy