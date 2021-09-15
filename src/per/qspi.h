#ifndef DSY_QSPI
#define DSY_QSPI /**< Macro */

#ifndef UNIT_TEST // for unit tests, a dummy implementation

#include <cstdint>
#include "daisy_core.h" // Added for dsy_gpio_pin typedef

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

    /** Indicates the current status of the module. 
         *  Warnings are indicated by a leading W.
         *  Errors are indicated by a leading E and cause an immediate exit.
         * 
         *  \param GOOD - No errors have been reported.
         *  \param E_HAL_ERROR - HAL code did not return HAL_OK.
         *  \param E_SWITCHING_MODES - An error was encountered while switching QSPI peripheral mode.
         *  \param E_INVALID_MODE - QSPI should not be written to while the program is executing from it.
         */
    enum Status
    {
        GOOD = 0,
        E_HAL_ERROR,
        E_SWITCHING_MODES,
        E_INVALID_MODE,
    };

    /** Configuration structure for interfacing with QSPI Driver */
    struct Config
    {
        /** Flash Devices supported. (Both of these are more-or-less the same, just different sizes). */
        enum Device
        {
            IS25LP080D,  /**< & */
            IS25LP064A,  /**< & */
            DEVICE_LAST, /**< & */
        };

        /** 
        Modes of operation.
        Memory Mapped mode: QSPI configured so that the QSPI can be
        read from starting address 0x90000000. Writing is not
        possible in this mode. \n 
        Indirect Polling mode: Device driver enabled. 
        */
        enum Mode
        {
            MEMORY_MAPPED,    /**< & */
            INDIRECT_POLLING, /**< & */
            MODE_LAST,
        };

        //SCK,  CE# (active low)
        struct
        {
            dsy_gpio_pin io0; /**< & */
            dsy_gpio_pin io1; /**< & */
            dsy_gpio_pin io2; /**< & */
            dsy_gpio_pin io3; /**< & */
            dsy_gpio_pin clk; /**< & */
            dsy_gpio_pin ncs; /**< & */
        } pin_config;

        Device device;
        Mode   mode;
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

    /** Returns the current class status. Useful for debugging.
     *  \returns Status
     */
    Status GetStatus();

    QSPIHandle() : pimpl_(nullptr) {}
    QSPIHandle(const QSPIHandle& other) = default;
    QSPIHandle& operator=(const QSPIHandle& other) = default;

    class Impl; /**< & */

  private:
    Impl* pimpl_;
};

/** @} */

} // namespace daisy

#else

#include <cstdint>
#include "../tests/TestIsolator.h"

namespace daisy
{
/** This is a dummy implementation for use in tests. 
 *  In your tests you can use this as a placeholder 
 *  for the physical volatile memory. 
 *  This provides a block of memory that can be erased, or written
 *  to.
 *  
 *  On the hardware, the memory can be read directly from addresses
 *  0x90000000 and above. However, within the unit tests you must use
 *  the Read() function.
 */
class QSPIHandle
{
  public:
    enum Result
    {
        OK = 0,
        ERR
    };

    static Result Write(uint32_t address, uint32_t size, uint8_t* buffer)
    {
        /** 256-byte aligned, normalized address value */
        uint32_t adjusted_addr = (address - kBaseAddress) & (uint32_t)(~0xff);
        std::copy(&buffer[adjusted_addr],
                  &buffer[adjusted_addr + size],
                  (uint8_t*)testIsolator_.GetStateForCurrentTest()->memory_);
        return Result::OK;
    }

    static Result Erase(uint32_t start_addr, uint32_t end_addr)
    {
        uint32_t adjusted_start_addr
            = (start_addr - kBaseAddress) & (uint32_t)(~0xff);
        uint32_t adjusted_end_addr
            = (end_addr - kBaseAddress) & (uint32_t)(~0xff);
        uint32_t* buff = testIsolator_.GetStateForCurrentTest()->memory_;
        // Erases memory by setting all bits to 1
        std::fill(
            &buff[adjusted_start_addr], &buff[adjusted_end_addr], 0xffffffff);
        return Result::OK;
    }

  private:
    static constexpr uint32_t kBaseAddress = 0x90000000;
    struct QSPIState
    {
        // 8MB pool of memory -- not sure if we should limit this,
        // but it will allow for testing out of range read/writes..
        // Also, I think this should probably be in the heap instead.
        uint32_t memory_[1024 * 1024 * 8 / 4] = {0xffffffff};
    };
    static TestIsolator<QSPIState> testIsolator_;
};


} // namespace daisy


#endif // ifndef UNIT_TEST

#endif // ifndef DSY_QSPI
