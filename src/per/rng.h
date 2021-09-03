#pragma once
#include "daisy_core.h"

namespace daisy
{
/**@brief True Random Number Generator access
 * @author shensley
 * @ingroup utility
 * 
 * Provides static access to the built-in True Random Number Generator
 */
class RandomNumberGenerator
{
  public:
    RandomNumberGenerator() {}
    ~RandomNumberGenerator() {}

    /** Initializes the Peripheral */
    static void Init();

    /** Deinitializes the Peripheral */
    static void DeInit();

    /** Returns a randomly generated 32-bit number
     *  This is done by polling the peripheral, 
     *  and can block for up to 100ms.
     * 
     *  To avoid blocking issues, the IsReady function can be 
     *  used to check if a value is ready before calling this function.
     *  
     *  If there is an issue with the peripheral, or a timeout occurs
     *  the return value will be 0.
     *  
     *  @return a 32-bit random number
     */
    static uint32_t GetValue();

    /** Checks the peripheral to see if a new value is ready 
     * 
     *  @return true if conditioning has finished and a new value can be read
    */
    static bool IsReady();
};

} // namespace daisy