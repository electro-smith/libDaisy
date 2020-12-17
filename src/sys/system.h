/** Low level System Configuration */
#ifndef DSY_SYSTEM_H
#define DSY_SYSTEM_H

#ifdef __cplusplus
#include <cstdint>

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

    /** Blocking Delay that uses the SysTick (1ms callback) to wait.
     ** \param delay_ms Time to delay in ms
     */
    static void Delay(uint32_t delay_ms);

    /**
	 ** Returns a const reference to the Systems Configuration struct
	 */
    const Config& GetConfig() const { return cfg_; }

  private:
    void   ConfigureClocks();
    void   ConfigureMpu();
    Config cfg_;
};
} // namespace daisy

#endif // ifdef __cplusplus


//#ifdef __cplusplus
//extern "C"
//{
//#endif
//#include <stdint.h>
//
//    /** @addtogroup system
//    @{
//    */
//
//    /** Initializes Clock tree, MPU, and internal memories voltage regulators.    
//    This function _must_ be called at the beginning of any program using libdaisy
//    Higher level daisy_ files call this through the DaisySeed object.
//    */
//    void dsy_system_init();
//
//    /** Jump to an address within the internal memory \n 
//       **This may not work correctly, and may not be very useful with the single sector of memory on the stm32h750**
//       \param addr Address to jump to
//    */
//    void dsy_system_jumpto(uint32_t addr);
//
//    /** Jumps to the first address of the external flash chip (0x90000000)
//    If there is no code there, the chip will likely fall through to the while() loop
//    TODO: Documentation/Loader for using external flash coming soon.
//    */
//    void dsy_system_jumptoqspi();
//
//    /** \return a uint32_t value of milliseconds since the SysTick started \n 
//    Note! This is a HAL_GetTick()    
//    */
//    uint32_t dsy_system_getnow();
//
//    /** Blocking Delay that uses the SysTick (1ms callback) to wait.
//    \param delay_ms Time to delay in ms
//    */
//    void dsy_system_delay(uint32_t delay_ms);
//
//#ifdef __cplusplus
//};
//#endif
#endif
/** @} */
