#ifndef DSY_QSPI
#define DSY_QSPI /**< Macro */
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "daisy_core.h" // Added for dsy_gpio_pin typedef

#define DSY_MEMORY_OK ((uint32_t)0x00) /**< & */
#define DSY_MEMORY_ERROR ((uint32_t)0x01) /**< & */

#define DSY_QSPI_TEXT       \
    __attribute__((section( \
        ".qspiflash_text"))) /**< used for reading memory in memory_mapped mode. */
#define DSY_QSPI_DATA       \
    __attribute__((section( \
        ".qspiflash_data"))) /**< used for reading memory in memory_mapped mode. */
#define DSY_QSPI_BSS        \
    __attribute__((section( \
        ".qspiflash_bss"))) /**< used for reading memory in memory_mapped mode. */

    /** @addtogroup serial
    @{
    */

    /** 
     Driver for QSPI peripheral to interface with external flash memory. \n 
     Currently supported QSPI Devices: \n 
     * IS25LP080D
    */


    //SCK,  CE# (active low)
    /** List of Pins used in QSPI (passed in during Init) */
    typedef enum
    {
        DSY_QSPI_PIN_IO0,  /**< & */
        DSY_QSPI_PIN_IO1,  /**< & */
        DSY_QSPI_PIN_IO2,  /**< & */
        DSY_QSPI_PIN_IO3,  /**< & */
        DSY_QSPI_PIN_CLK,  /**< & */
        DSY_QSPI_PIN_NCS,  /**< & */
        DSY_QSPI_PIN_LAST, /**< & */
    } dsy_qspi_pin;

    /** 
    Modes of operation.
    Memory Mapped mode: QSPI configured so that the QSPI can be
    read from starting address 0x90000000. Writing is not
    possible in this mode. \n 
    Indirect Polling mode: Device driver enabled. \n     
    Read/Write possible via dsy_qspi_* functions
    */
    typedef enum
    {
        DSY_QSPI_MODE_DSY_MEMORY_MAPPED, /**< & */
        DSY_QSPI_MODE_INDIRECT_POLLING,  /**< & */
        DSY_QSPI_MODE_LAST,              /**< & */
    } dsy_qspi_mode;


    /** Flash Devices supported. (Both of these are more-or-less the same, just different sizes). */
    typedef enum
    {
        DSY_QSPI_DEVICE_IS25LP080D, /**< & */
        DSY_QSPI_DEVICE_IS25LP064A, /**< & */
        DSY_QSPI_DEVICE_LAST,       /**< & */
    } dsy_qspi_device;

    /** Configuration structure for interfacing with QSPI Driver */
    typedef struct
    {
        dsy_qspi_mode   mode;                          /**< & */
        dsy_qspi_device device;                        /**< & */
        dsy_gpio_pin    pin_config[DSY_QSPI_PIN_LAST]; /**< & */
    } dsy_qspi_handle;

    /** 
    Initializes QSPI peripheral, and Resets, and prepares memory for access.
    \param hqspi should be populated with the mode, device and pin_config before calling this function.
    \return DSY_MEMORY_OK or DSY_MEMORY_ERROR
    */
    int dsy_qspi_init(dsy_qspi_handle* hqspi);

    /** 
    Deinitializes the peripheral
    This should be called before reinitializing QSPI in a different mode.
    \return DSY_MEMORY_OK or DSY_MEMORY_ERROR
    */
    int dsy_qspi_deinit();

    /** 
    Writes a single page to to the specified address on the QSPI chip.
    For IS25LP* page size is 256 bytes.
    \param adr Address to write to
    \param sz Buff size
    \param buf Buffer to write
    \return DSY_MEMORY_OK or DSY_MEMORY_ERROR
    */
    int dsy_qspi_writepage(uint32_t adr, uint32_t sz, uint8_t* buf);

    /** 
    Writes data in buffer to to the QSPI. Starting at address to address+size 
    \param address Address to write to
    \param size Buffer size
    \param buffer Buffer to write
    \return DSY_MEMORY_OK or DSY_MEMORY_ERROR
     */
    int dsy_qspi_write(uint32_t address, uint32_t size, uint8_t* buffer);

    /** 
    Erases the area specified on the chip.
    Erasures will happen by 4K, 32K or 64K increments.
    Smallest erase possible is 4kB at a time. (on IS25LP*)
    \param start_adr Address to begin erasing from
    \param end_adr  Address to stop erasing at
    \return DSY_MEMORY_OK or DSY_MEMORY_ERROR
    */
    int dsy_qspi_erase(uint32_t start_adr, uint32_t end_adr);

    /**  
      Erases a single sector of the chip.  
      TODO: Document the size of this function. 
      \param addr Address of sector to erase
      \return DSY_MEMORY_OK or DSY_MEMORY_ERROR
     */
    int dsy_qspi_erasesector(uint32_t addr);


#ifdef __cplusplus
}
#endif

#endif
/** @} */
