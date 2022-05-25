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
    Result DeInit();

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
        \param address Address of sector to erase
        \return Result::OK or Result::ERR
        */
    Result EraseSector(uint32_t address);

    /** Returns the current class status. Useful for debugging.
     *  \returns Status
     */
    Status GetStatus();

    /** Returns a pointer to the actual memory used 
     *  The memory at this address is read-only
     *  to write to it use the Write function.
     * 
     *  \param offset returns the pointer starting this 
     *                many bytes into the memory
    */
    void* GetData(uint32_t offset = 0);

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
 */
class QSPIHandle
{
  public:
    enum Result
    {
        OK = 0,
        ERR
    };

    /** A mock-only function for resetting the memory to clean state 
     *  This should be called at the beginning of any test to ensure that
     *  data from a previous test does not interfere.
     */
    static Result ResetAndClear()
    {
        testIsolator_.GetStateForCurrentTest()->memory_.clear();
        return Result::OK;
    }


    static Result Write(uint32_t address, uint32_t size, uint8_t* buffer)
    {
        // 256-byte aligned, normalized address value
        uint32_t adjusted_addr = (address) & (uint32_t)(~0xff);
        // Make sure memory is of approriate size
        uint32_t total_bytes = adjusted_addr + size;
        AdaptToSize(total_bytes);
        // Copy data into vector
        uint8_t* dest = testIsolator_.GetStateForCurrentTest()->memory_.data();
        std::copy(&buffer[adjusted_addr],
                  &buffer[adjusted_addr + size],
                  &dest[adjusted_addr]);
        return Result::OK;
    }

    static Result Erase(uint32_t start_addr, uint32_t end_addr)
    {
        uint32_t adjusted_start_addr = (start_addr) & (uint32_t)(~0xff);
        uint32_t adjusted_end_addr   = (end_addr) & (uint32_t)(~0xff);

        // guard addresses
        assert(adjusted_start_addr < kMaxAdjustedAddr);
        assert(adjusted_end_addr < kMaxAdjustedAddr);

        // Make sure vector is of appropriate size
        // size should be at least (adjusted_end_addr)
        AdaptToSize(adjusted_end_addr);
        uint8_t* buff = testIsolator_.GetStateForCurrentTest()->memory_.data();
        // Erases memory by setting all bits to 1
        std::fill(&buff[adjusted_start_addr], &buff[adjusted_end_addr], 0xff);
        return Result::OK;
    }

    /** Returns a pointer to the actual memory used 
    */
    static void* GetData(uint32_t offset = 0)
    {
        assert(offset < kMaxAdjustedAddr);
        AdaptToSize(offset + 1); /**< Make sure it's not empty */
        return (void*)(testIsolator_.GetStateForCurrentTest()->memory_.data()
                       + offset);
    }

    /** Returns the current size of the memory vector.
     *  
     *  This is not in the hardware class its just for testing purposes
     */
    static size_t GetCurrentSize()
    {
        return testIsolator_.GetStateForCurrentTest()->memory_.size();
    }

  private:
    /** Adjusts the test state vector to an appropriate size */
    static void AdaptToSize(uint32_t required_bytes)
    {
        if(testIsolator_.GetStateForCurrentTest()->memory_.size()
           < required_bytes)
            testIsolator_.GetStateForCurrentTest()->memory_.resize(
                required_bytes, 0x00);
    }
    static constexpr uint32_t kMaxAdjustedAddr = 0x800000;
    struct QSPIState
    {
        // Emulate the byte-memory of the QSPI flash
        std::vector<uint8_t> memory_;
    };
    static TestIsolator<QSPIState> testIsolator_;
};


} // namespace daisy


#endif // ifndef UNIT_TEST

#endif // ifndef DSY_QSPI
