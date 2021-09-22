/** SDRAM for 32MB AS4C16M16SA (and 64MB equivalent).
    Thanks to whoever this awesome person is:
    http://main.lv/writeup/stm32f4_sdram_configuration.md
    The Init function is basically a copy and paste.
    He has references to timing, etc.
    RAM is configured at 100MHz (fastest possible on the MCU).
    To use these the .sdram_data/_bss sections must be
    configured correctly in the LINKER SCRIPT.
    using BSS is advised for most things, since the
    DATA section must also fit in flash in order to be initialized.
    Data section init not properly set up, as SDRAM is not initialized until after startup code.
*/

//Edit the Linker Script to have the following lines in the following sections:
/*
MEMORY
{
    SDRAM (RWX) : ORIGIN = 0xC0000000, LENGTH = 64M
}
SECTIONS
{

    .sdram_data :
    {
        . = ALIGN(4);
        _ssdram_data = .;


        PROVIDE(__sdram_data_start = _ssdram_data);
        *(.sdram_data)
        *(.sdram_data*)
        . = ALIGN(4);
        _esdram_data = .;

        PROVIDE(__sdram_data_end = _esdram_data);
     } > SDRAM AT >FLASH
     _sisdram_data = LOADADDR(.sdram_data);


    .sdram_bss (NOLOAD):
    {
        . = ALIGN(4);
        __ssdram_bss = .;
        PROVIDE(__sdram_bss_start = _ssdram_bss);
        *(.sdram_bss)
        *(.sdram_bss*)
        . = ALIGN(4);
        _esdram_bss = .;


        PROVIDE(__sdram_bss_end = _esdram_bss);

     } > SDRAM
}
*/
#ifndef RAM_AS4C16M16SA_H
#define RAM_AS4C16M16SA_H /**< & */
#include <stdint.h>
#include "daisy_core.h"

/** @addtogroup sdram
    @{
    */


/*
    As mentioned above, this does not currently initialize correctly (startup
    code needs to be modified to init SDRAM, and fill
    The variables placed here will also need to fit inside of the flash in order to initialize.
  */

/** Usage: \n 
  E.g. int DSY_SDRAM_DATA initialized_var = 1; */
#define DSY_SDRAM_DATA __attribute__((section(".sdram_data")))


/** Variables placed here will not be initialized. \n 
    Usage \n 
    E.g. int DSY_SDRAM_BSS uninitialized_var;
*/
#define DSY_SDRAM_BSS __attribute__((section(".sdram_bss")))

class SdramHandle
{
  public:
    enum class Result
    {
        OK,  /**< & */
        ERR, /**< & */
    };

    /** Initializes the SDRAM peripheral */
    Result Init();
    Result DeInit();

  private:
    Result PeriphInit();
    Result DeviceInit();
    Result PeriphDeInit();
    Result DeviceDeInit();
};
#endif
/** @} */
