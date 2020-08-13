#include <stm32h7xx_hal.h>
#include "sys/system.h"
#include "sys/dma.h"

// global init functions for peripheral drivers.
extern void dsy_i2c_global_init();

// Jump related stuff

#define u32 uint32_t
#define vu32 volatile uint32_t
#define SET_REG(addr, val)    \
    do                        \
    {                         \
        *(vu32*)(addr) = val; \
    } while(0)
#define GET_REG(addr) (*(vu32*)(addr))

#define RCC_CR RCC
#define RCC_CFGR (RCC + 0x04)
#define RCC_CIR (RCC + 0x08)
#define RCC_AHBENR (RCC + 0x14)
#define RCC_APB2ENR (RCC + 0x18)
#define RCC_APB1ENR (RCC + 0x1C)

#define SCS 0xE000E000
#define STK (SCS + 0x10)
#define STK_CTRL (STK + 0x00)
#define RCC_CR RCC

typedef struct
{
    vu32 ISER[2];
    u32  RESERVED0[30];
    vu32 ICER[2];
    u32  RSERVED1[30];
    vu32 ISPR[2];
    u32  RESERVED2[30];
    vu32 ICPR[2];
    u32  RESERVED3[30];
    vu32 IABR[2];
    u32  RESERVED4[62];
    vu32 IPR[15];
} NVIC_TypeDef;

__attribute__((always_inline)) static inline void __JUMPTOQSPI()
{
    __asm("LDR R1, =0xE000ED00;"); // SCB
    __asm("LDR R0, =0x90000000;"); // APP BASE
    __asm("STR R0, [R1, #8]");     // VTOR
    __asm("LDR SP, [R0, #0]");     // SP @ +0
    __asm("LDR R0, [R0, #4]");     // PC @ +4
    __asm("BX R0");
}

typedef void (*EntryPoint)(void);

// Static Function Declaration
static void SystemClock_Config();
static void MPU_Config();
static void Error_Handler(void);
void        CubeClockConfig();

void SysTick_Handler(void)
{
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
}

void HardFault_Handler()
{
    asm("bkpt 255");
}

void dsy_system_init()
{
    HAL_Init();
    SystemClock_Config();
    MPU_Config();
    dsy_dma_init();
    dsy_i2c_global_init();
    SCB_EnableICache();
    SCB_EnableDCache();
}

void dsy_system_jumptoqspi()
{
    __JUMPTOQSPI();
    while(1) {}
}

uint32_t dsy_system_getnow()
{
    return HAL_GetTick();
}

void dsy_system_delay(uint32_t delay_ms)
{
    HAL_Delay(delay_ms);
}


void SystemClock_Config()
{
    // HOLD THISSSS
    RCC_OscInitTypeDef       RCC_OscInitStruct   = {0};
    RCC_ClkInitTypeDef       RCC_ClkInitStruct   = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Supply configuration update enable 
  */
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
    /** Configure the main internal regulator output voltage 
  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    //    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

    while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
    /** Macro to configure the PLL clock source 
  */
    __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
    /** Initializes the CPU, AHB and APB busses clocks 
  */
    RCC_OscInitStruct.OscillatorType
        = RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState      = RCC_HSE_ON;
    RCC_OscInitStruct.HSI48State    = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState  = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM      = 4;
    RCC_OscInitStruct.PLL.PLLN      = 200;
    //    RCC_OscInitStruct.PLL.PLLN      = 240;
    RCC_OscInitStruct.PLL.PLLP      = 2;
    RCC_OscInitStruct.PLL.PLLQ      = 5; // was 4 in cube
    RCC_OscInitStruct.PLL.PLLR      = 2;
    RCC_OscInitStruct.PLL.PLLRGE    = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN  = 0;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Initializes the CPU, AHB and APB busses clocks 
  */
    RCC_ClkInitStruct.ClockType
        = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1
          | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
    PeriphClkInitStruct.PeriphClockSelection
        = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_RNG | RCC_PERIPHCLK_SPI1
          | RCC_PERIPHCLK_SAI2 | RCC_PERIPHCLK_SAI1 | RCC_PERIPHCLK_SDMMC
          | RCC_PERIPHCLK_I2C2 | RCC_PERIPHCLK_ADC | RCC_PERIPHCLK_I2C1
          | RCC_PERIPHCLK_USB | RCC_PERIPHCLK_QSPI | RCC_PERIPHCLK_FMC;
    // PLL 2
    PeriphClkInitStruct.PLL2.PLL2M = 4;
    //  PeriphClkInitStruct.PLL2.PLL2N = 115; // Max Freq @ 3v3
    PeriphClkInitStruct.PLL2.PLL2N      = 84; // Max Freq @ 1V9
    PeriphClkInitStruct.PLL2.PLL2P      = 8;  // 57.5
    PeriphClkInitStruct.PLL2.PLL2Q      = 10; // 46
    PeriphClkInitStruct.PLL2.PLL2R      = 2;  // 115Mhz
    PeriphClkInitStruct.PLL2.PLL2RGE    = RCC_PLL2VCIRANGE_2;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    PeriphClkInitStruct.PLL2.PLL2FRACN  = 0;
    // PLL 3
    PeriphClkInitStruct.PLL3.PLL3M        = 6;
    PeriphClkInitStruct.PLL3.PLL3N        = 295;
    PeriphClkInitStruct.PLL3.PLL3P        = 64; // 12.29Mhz
    PeriphClkInitStruct.PLL3.PLL3Q        = 4;
    PeriphClkInitStruct.PLL3.PLL3R        = 32; // 24.xMhz
    PeriphClkInitStruct.PLL3.PLL3RGE      = RCC_PLL3VCIRANGE_1;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL   = RCC_PLL3VCOWIDE;
    PeriphClkInitStruct.PLL3.PLL3FRACN    = 0;
    PeriphClkInitStruct.FmcClockSelection = RCC_FMCCLKSOURCE_PLL2;
    //PeriphClkInitStruct.QspiClockSelection = RCC_QSPICLKSOURCE_PLL2;
    PeriphClkInitStruct.QspiClockSelection   = RCC_QSPICLKSOURCE_D1HCLK;
    PeriphClkInitStruct.SdmmcClockSelection  = RCC_SDMMCCLKSOURCE_PLL;
    PeriphClkInitStruct.Sai1ClockSelection   = RCC_SAI1CLKSOURCE_PLL3;
    PeriphClkInitStruct.Sai23ClockSelection  = RCC_SAI23CLKSOURCE_PLL3;
    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
    PeriphClkInitStruct.Usart234578ClockSelection
        = RCC_USART234578CLKSOURCE_D2PCLK1;
    //PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
    //PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_PLL2;
    PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
    //PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_PLL3;
    PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_D2PCLK1;
    PeriphClkInitStruct.I2c4ClockSelection   = RCC_I2C4CLKSOURCE_PLL3;
    //PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_D3PCLK1;
    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
    //PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL3;
    PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL3;
    if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Enable USB Voltage detector 
  */
    HAL_PWREx_EnableUSBVoltageDetector();
}

static void MPU_Config()
{
    MPU_Region_InitTypeDef MPU_InitStruct;
    HAL_MPU_Disable();
    // Configure RAM D2 (SRAM1) as non cacheable
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress      = 0x30000000;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_16KB;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    MPU_InitStruct.IsCacheable  = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
    MPU_InitStruct.IsShareable  = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number       = MPU_REGION_NUMBER1;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.Size         = MPU_REGION_SIZE_64MB;
    MPU_InitStruct.BaseAddress  = 0xC0000000;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);

    //	uint32_t enable_bit = (1 << 0);
    //	uint32_t privdefena = (1 << 2);
    //	uint32_t rasr_enable = (1 << 0);


    //	MPU->CTRL = enable_bit | privdefena;
    //	MPU->RNR  = 0x00;
    //	MPU->RBAR = 0x30000000;
    //	MPU->RASR = rasr_enable;


    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

static void Error_Handler()
{
    // Insert code to handle errors here.
    while(1) {}
}
