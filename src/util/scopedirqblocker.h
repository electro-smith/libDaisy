#pragma once

#include <stdint.h>

#ifndef UNIT_TEST // provide dummy implementation for unit tests
#include <cmsis_gcc.h>

namespace daisy
{
/** Tempoaraily disables IRQ handlers with RAII techniques. */
class ScopedIrqBlocker
{
  public:
    ScopedIrqBlocker()
    {
        prim_ = __get_PRIMASK();
        __disable_irq();
    }

    ~ScopedIrqBlocker()
    {
        if(!prim_)
            __enable_irq();
    }

  private:
    uint32_t prim_;
};
} // namespace daisy

#else // ifndef UNIT_TEST

namespace daisy
{
/** A dummy implementation for unit tests */
class ScopedIrqBlocker
{
  public:
    ScopedIrqBlocker(){};
    ~ScopedIrqBlocker() = default;
};
} // namespace daisy

#endif
