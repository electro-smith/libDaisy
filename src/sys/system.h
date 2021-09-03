#ifndef DSY_SYSTEM_H
#define DSY_SYSTEM_H

#ifndef UNIT_TEST // for unit tests, a dummy implementation is provided below

#include <cstdint>
#include "per/tim.h"

namespace daisy
{
/** A handle for interacting with the Core System. 
 ** This includes the Clock tree, MPU, global DMA initialization,
 ** cache handling, and any other necessary global initiailzation
 **
 ** @author shensley
 ** @addtogroup system
 */
class System
{
  public:
    /** Contains settings for initializing the System */
    struct Config
    {
        /** Specifies the system clock frequency that feeds APB/AHB clocks, etc. */
        enum class SysClkFreq
        {
            FREQ_400MHZ,
            FREQ_480MHZ,
        };

        /** Method to call on the struct to set to defaults
         ** CPU Freq set to 400MHz
         ** Cache Enabled 
         ** */
        void Defaults()
        {
            cpu_freq   = SysClkFreq::FREQ_400MHZ;
            use_dcache = true;
            use_icache = true;
        }

        /** Method to call on the struct to set to boost mode:
         ** CPU Freq set to 480MHz
         ** Cache Enabled 
         ** */
        void Boost()
        {
            cpu_freq   = SysClkFreq::FREQ_480MHZ;
            use_dcache = true;
            use_icache = true;
        }

        SysClkFreq cpu_freq;
        bool       use_dcache;
        bool       use_icache;
    };

    /** A simple way to represent where the program is executing from
     * 
     */
    enum ProgramMemory
    {
        INTERNAL_FLASH = 0,
        AXI_SRAM,
        QSPI,
        INVALID_ADDRESS,
    };

    System() {}
    ~System() {}

    /** Default Initializer with no input will create an internal config, 
     ** and set everything to Defaults
     */
    void Init();

    /** Configurable Initializer
     ** Initializes clock tree, DMA initializaiton and 
     ** any necessary global inits.
     */
    void Init(const Config& config);

    /** Jumps to the first address of the external flash chip (0x90000000)
     ** If there is no code there, the chip will likely fall through to the while() loop
     ** TODO: Documentation/Loader for using external flash coming soon.
     */
    void JumpToQspi();

    /** \return a uint32_t value of milliseconds since the SysTick started 
    */
    static uint32_t GetNow();

    /** \return a uint32_t of microseconds within the internal timer. */
    static uint32_t GetUs();

    /** \return a uint32_t of ticks at (PCLk1 * 2)Hz 
     ** Useful for measuring the number of CPU ticks 
     ** something is taking.
     ** */
    static uint32_t GetTick();

    /** Blocking Delay that uses the SysTick (1ms callback) to wait.
     ** \param delay_ms Time to delay in ms
     */
    static void Delay(uint32_t delay_ms);

    /** Blocking Delay using internal timer to wait 
     ** \param delay_us Time to ddelay in microseconds */
    static void DelayUs(uint32_t delay_us);


    /** Blocking Delay using internal timer to wait 
     ** \param delay_ticks Time to ddelay in microseconds */
    static void DelayTicks(uint32_t delay_ticks);

    /** Triggers a reset of the seed and starts in bootloarder
     ** mode to allow firmware update. */
    static void ResetToBootloader();

    /** Returns the tick rate in Hz with which GetTick() is incremented. */
    static uint32_t GetTickFreq();

    /** Returns the Frequency of the system clock in Hz 
     ** This is the primary system clock that is used to generate
     ** AXI Peripheral, APB, and AHB clocks. */
    static uint32_t GetSysClkFreq();

    /** Returns the frequency of the HCLK (AHB) clock. This is derived
     ** from the System clock, and used to clock the CPU, memory, and
     ** peripherals mapped on the AHB, and APB Bus.
     ** */
    static uint32_t GetHClkFreq();

    /** Returns the frequency of the PCLK1 (APB1) clock
     ** This is used to clock various peripherals, and timers.
     **
     ** It's  important to  note that many timers run on a 
     ** clock twice as fast as the peripheral clock for the timer. 
     ** */
    static uint32_t GetPClk1Freq();

    /** Returns the frequency of the PCLK2 (APB2) clock
     ** This is used to clock various peripherals, and timers.
     **
     ** It's  important to  note that many timers run on a 
     ** clock twice as fast as the peripheral clock for the timer. 
     ** */
    static uint32_t GetPClk2Freq();

    /**
     ** Returns a const reference to the Systems Configuration struct
     */
    const Config& GetConfig() const { return cfg_; }

    /** Returns an enum representing the current (primary) memory space used
     *  for executing the program.
     */
    static ProgramMemory GetProgramMemory();

  private:
    void   ConfigureClocks();
    void   ConfigureMpu();
    Config cfg_;

    // TODO -- unify this with bootloader so we don't have reduntant values
    static constexpr uint32_t sram_start_ = 0x24000000U;
    static constexpr uint32_t sram_end_   = sram_start_ + 0x80000U;
    static constexpr uint32_t qspi_start_ = 0x90040000U;
    // TODO -- this is a bit too large:
    static constexpr uint32_t qspi_end_       = qspi_start_ + 0x800000U;
    static constexpr uint32_t internal_start_ = 0x08000000U;
    static constexpr uint32_t internal_end_   = internal_start_ + 0x20000U;

    /** One TimerHandle to rule them all
     ** Maybe this whole class should be static.. */
    static TimerHandle tim_;
};
} // namespace daisy

#else // ifndef UNIT_TEST

#include <cstdint>
#include "../tests/TestIsolator.h"
namespace daisy
{
/** This is a dummy implementation for use in unit tests.
 *  In your test, you can set the current system time to
 *  control the "flow of time" :-)
 *  Only the time-related functions are added here. If
 *  your tests need some of the other functions, feel
 *  free to add them here as well.
 * 
 *  To decouple tests that are running in parallel, each
 *  test can independently modify the current time.
 */
class System
{
  public:
    static uint32_t GetNow()
    {
        return testIsolator_.GetStateForCurrentTest()->currentUs_ / 1000;
    }
    static uint32_t GetUs()
    {
        return testIsolator_.GetStateForCurrentTest()->currentUs_;
    }
    static uint32_t GetTick()
    {
        return testIsolator_.GetStateForCurrentTest()->currentTick_;
    }
    static uint32_t GetTickFreq()
    {
        return testIsolator_.GetStateForCurrentTest()->tickFreqHz_;
    }

    /** Sets the current "tick" value for the test that's currently running. */
    static void SetTickForUnitTest(uint32_t tick)
    {
        testIsolator_.GetStateForCurrentTest()->currentTick_ = tick;
    }
    /** Sets the current microsecond value for the test that's currently running. */
    static void SetUsForUnitTest(uint32_t us)
    {
        testIsolator_.GetStateForCurrentTest()->currentUs_ = us;
    }
    /** Sets the tick frequency for the test that's currently running. */
    static void SetTickFreqForUnitTest(uint32_t freqInHz)
    {
        testIsolator_.GetStateForCurrentTest()->tickFreqHz_ = freqInHz;
    }

  private:
    struct SystemState
    {
        uint32_t currentTick_ = 0;
        uint32_t currentUs_   = 0;
        uint32_t tickFreqHz_  = 0;
    };
    static TestIsolator<SystemState> testIsolator_;
};

} // namespace daisy

#endif // ifndef UNIT_TEST
#endif
