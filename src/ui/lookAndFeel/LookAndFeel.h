#pragma once
#include <cmath>
#include "hid/disp/graphics_common.h"
#include "hid/disp/display.h"

namespace daisy
{
/** Implements drawing routines for menus, sliders and other common tasks. For each class that uses 
 *  the OneBitGraphicsLookAndFeel, corresponding drawing functions are provided by inheritance.
 * 
 *  To create your custom looking UI, inherit from this class and overwrite the member functions 
 *  as needed. Then assign an object of your new class to a UI or to individual UiPages.
 */
class OneBitGraphicsLookAndFeel
{
  public:
    virtual ~OneBitGraphicsLookAndFeel() {}
};
} // namespace daisy