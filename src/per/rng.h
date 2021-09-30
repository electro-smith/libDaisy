#pragma once
#include "daisy_core.h"

namespace daisy
{
/** @brief True Random Number Generator access
 *  @author shensley
 *  @ingroup utility
 * 
 *  Provides static access to the built-in True Random Number Generator
 */
class Random
{
  public:
    Random() {}
    ~Random() {}

    /** Initializes the Peripheral
     * 
     *  This is called from System::Init, 
     *  so the GetValue, and GetFloat functions
     *  can be used without the application needing to 
     *  manually initialize the RNG. 
     */
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

    /** Returns a floating point value between the specified
     *  minimum and maximum. Calls GetValue() internally. 
     * 
     *  \param min the minimum value to return, defaults to 0.f
     *  \param max the maximum value to return, defaults to 1.f
     */
    static float GetFloat(float min = 0.f, float max = 1.f);

    /** Checks the peripheral to see if a new value is ready 
     * 
     *  @return true if conditioning has finished and a new value can be read
    */
    static bool IsReady();
};

} // namespace daisy