#ifndef DSY_QSPI
#define DSY_QSPI /**< Macro */

#include <cstdint>
#include "per/gpio.h"

#define DSY_QSPI_TEXT       \
    __attribute__((section( \
        ".qspiflash_text"))) /**< used for reading memory in memory_mapped mode. */
#define DSY_QSPI_DATA       \
    __attribute__((section( \
        ".qspiflash_data"))) /**< used for reading memory in memory_mapped mode. */
#define DSY_QSPI_BSS        \
    __attribute__((section( \
        ".qspiflash_bss"))) /**< used for reading memory in memory_mapped mode. */

namespace daisy
{
/** @addtogroup serial
@{
*/

/** 
 Driver for QSPI peripheral to interface with external flash memory. \n 
    Currently supported QSPI Devices: \n 
    * IS25LP080D
*/
class QSPIHandle
{
  public:
    enum Result
    {
        OK = 0,
        ERR
    };

    /** Configuration structure for interfacing with QSPI Driver */
    struct Config
    {
        /** 
            Modes of operation.
            Memory Mapped mode: QSPI configured so that the QSPI can be
            read from starting address 0x90000000. Writing is not
            possible in this mode. \n 
            Indirect Polling mode: Device driver enabled. \n     
            Read/Write possible via dsy_qspi_* functions
            */
        enum Mode
        {
            DSY_MEMORY_MAPPED, /**< & */
            INDIRECT_POLLING,  /**< & */
            MODE_LAST,
        };

        /** Flash Devices supported. (Both of these are more-or-less the same, just different sizes). */
        enum Device
        {
            IS25LP080D,  /**< & */
            IS25LP064A,  /**< & */
            DEVICE_LAST, /**< & */
        };

        //SCK,  CE# (active low)
        struct
        {
            Pin io0; /**< & */
            Pin io1; /**< & */
            Pin io2; /**< & */
            Pin io3; /**< & */
            Pin clk; /**< & */
            Pin ncs; /**< & */
        } pin_config;

        Mode   mode;
        Device device;
    };

    /** 
        Initializes QSPI peripheral, and Resets, and prepares memory for access.
        \param config should be populated with the mode, device and pin_config before calling this function.
        \return Result::OK or Result::ERR
        */
    Result Init(const Config& config);

    /** Returns the current config. */
    const Config& GetConfig() const;

    // Couldn't this just be called before anything else in init? That
    // would make manually calling it unnecessary.
    /** 
        Deinitializes the peripheral
        This should be called before reinitializing QSPI in a different mode.
        \return Result::OK or Result::ERR
        */
    Result Deinit();

    /** 
        Writes a single page to to the specified address on the QSPI chip.
        For IS25LP*, page size is 256 bytes.
        \param address Address to write to
        \param size Buffer size
        \param buffer Buffer to write
        \return Result::OK or Result::ERR
        */
    Result WritePage(uint32_t address, uint32_t size, uint8_t* buffer);

    /** 
        Writes data in buffer to to the QSPI. Starting at address to address+size 
        \param address Address to write to
        \param size Buffer size
        \param buffer Buffer to write
        \return Result::OK or Result::ERR
        */
    Result Write(uint32_t address, uint32_t size, uint8_t* buffer);

    /** 
        Erases the area specified on the chip.
        Erasures will happen by 4K, 32K or 64K increments.
        Smallest erase possible is 4kB at a time. (on IS25LP*)
        \param start_addr Address to begin erasing from
        \param end_addr  Address to stop erasing at
        \return Result::OK or Result::ERR
        */
    Result Erase(uint32_t start_addr, uint32_t end_addr);

    /**  
         Erases a single sector of the chip.  
        TODO: Document the size of this function. 
        \param addr Address of sector to erase
        \return Result::OK or Result::ERR
        */
    Result EraseSector(uint32_t address);

    QSPIHandle() : pimpl_(nullptr) {}
    QSPIHandle(const QSPIHandle& other) = default;
    QSPIHandle& operator=(const QSPIHandle& other) = default;

    class Impl; /**< & */

  private:
    Impl* pimpl_;
};

/** @} */

} // namespace daisy

#endif
