#pragma once

#include <stdint.h>
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