#include <stdint.h>
#include <stm32h7xx_hal.h>
#include "util/hal_map.h"

#define SDRAM_MODEREG_BURST_LENGTH_2 ((1 << 0))
#define SDRAM_MODEREG_BURST_LENGTH_4 ((1 << 1))

#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL ((0 << 3))

#define SDRAM_MODEREG_CAS_LATENCY_3 ((1 << 4) | (1 << 5))

#define SDRAM_MODEREG_OPERATING_MODE_STANDARD ()

#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE ((1 << 9))
#define SDRAM_MODEREG_WRITEBURST_MODE_PROG_BURST ((0 << 9))

typedef struct
{
  uint8_t             board;
  SDRAM_HandleTypeDef hsdram;
} dsy_sdram_t;

void SdramInit();
void PeriphInit();
void DeviceInit();
void SdramMpuInit();

#define TICKS_PER_US 64e6f * 1e-6f

void SdramInit()
{
  PeriphInit();
  DeviceInit();
  SdramMpuInit();
}

static dsy_sdram_t dsy_sdram;

// These functions rely on the DWT unit, which
// seems unreliable without a debugger attached
// void EnableProcessTick()
// {
//     DWT->CTRL = 1;
// }

// float GetProcessTickUs()
// {
//     return (float) DWT->CYCCNT / TICKS_PER_US;
// }

// float GetProcessTickMs()
// {
//     return GetProcessTickUs() / 1e3f;
// }

// void DelayProcessUs(float microseconds)
// {
//   DWT->CTRL = 1; // ensure enable CYCCNT bit

//   uint32_t start = DWT->CYCCNT;

//   // This assumes we're using the HSI oscillator, which
//   // for the STM32H750 is 64MHz

//   uint32_t num_ticks = microseconds * TICKS_PER_US;

//   while (DWT->CYCCNT - start < num_ticks);

//   DWT->CYCCNT = 0; 

// //   // Disabling the counter after its use
// //   DWT->CTRL = 0;
// }

// void DelayProcessMs(float milliseconds)
// {
//     DelayProcessUs(milliseconds * 1e3f);
// }

void PeriphInit()
{
  FMC_SDRAM_TimingTypeDef SdramTiming = {0};
  dsy_sdram.hsdram.Instance           = FMC_SDRAM_DEVICE;
  // Init
  dsy_sdram.hsdram.Init.SDBank             = FMC_SDRAM_BANK1;
  dsy_sdram.hsdram.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_9;
  dsy_sdram.hsdram.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_13;
  dsy_sdram.hsdram.Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_32;
  dsy_sdram.hsdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  dsy_sdram.hsdram.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
  dsy_sdram.hsdram.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  dsy_sdram.hsdram.Init.SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_2;
  dsy_sdram.hsdram.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
  dsy_sdram.hsdram.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;
  /* SdramTiming */
  SdramTiming.LoadToActiveDelay    = 2;
  SdramTiming.ExitSelfRefreshDelay = 7;
  SdramTiming.SelfRefreshTime      = 4;
  SdramTiming.RowCycleDelay        = 8; // started at 7
  SdramTiming.WriteRecoveryTime    = 3;
  SdramTiming.RPDelay              = 0;
  SdramTiming.RCDDelay             = 10; // started at 2
  //	SdramTiming.LoadToActiveDelay = 16;
  //	SdramTiming.ExitSelfRefreshDelay = 16;
  //	SdramTiming.SelfRefreshTime = 16;
  //	SdramTiming.RowCycleDelay = 16;
  //	SdramTiming.WriteRecoveryTime = 16;
  //	SdramTiming.RPDelay = 16;
  //	SdramTiming.RCDDelay = 16;

  // TODO -- add some kind of error indication
  if(HAL_SDRAM_Init(&dsy_sdram.hsdram, &SdramTiming) != HAL_OK)
  {
      //Error_Handler();
      // return Result::ERR;
  }
  // return Result::OK;
}

void DeviceInit()
{
  FMC_SDRAM_CommandTypeDef Command;

  __IO uint32_t tmpmrd = 0;
  /* Step 3:  Configure a clock configuration enable command */
  Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(&dsy_sdram.hsdram, &Command, 0x1000);

  /* Step 4: Insert 100 ms delay */
  // HAL_Delay(100);
  // DelayProcessMs(100);

  // TODO -- double check that this takes the appropriate amount of time (seems a bit long)
  // Volatile to (hopefully) ensure no optimization occurs. This may need to be accompanied with
  // pragmas and attributes to be compiler cross-compatible

  // The startup clock is 64MHz, and this loop
  // typically compiles to 6 instructions, so for
  // a delay of ~100 ms...
  for (volatile int i = 0; i < (int) (64e5 / 6); i++);

  /* Step 5: Configure a PALL (precharge all) command */
  Command.CommandMode            = FMC_SDRAM_CMD_PALL;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(&dsy_sdram.hsdram, &Command, 0x1000);

  /* Step 6 : Configure a Auto-Refresh command */
  Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 4;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(&dsy_sdram.hsdram, &Command, 0x1000);

  /* Step 7: Program the external memory mode register */
  tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_4
            | SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL | SDRAM_MODEREG_CAS_LATENCY_3
            | SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
  //SDRAM_MODEREG_OPERATING_MODE_STANDARD | // Used in example, but can't find reference except for "Test Mode"

  Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = tmpmrd;

  /* Send the command */
  HAL_SDRAM_SendCommand(&dsy_sdram.hsdram, &Command, 0x1000);

  //HAL_SDRAM_ProgramRefreshRate(hsdram, 0x56A - 20);
  HAL_SDRAM_ProgramRefreshRate(&dsy_sdram.hsdram, 0x81A - 20);
  // return Result::OK;
}

static uint32_t FMC_Initialized = 0;

static void HAL_FMC_MspInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(FMC_Initialized)
    {
        return;
    }
    FMC_Initialized = 1;

    /* Peripheral clock enable */
    __HAL_RCC_FMC_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    // for SDNWE on some boards:
    __HAL_RCC_GPIOC_CLK_ENABLE();


    /** FMC GPIO Configuration  
	PE1   ------> FMC_NBL1
	PE0   ------> FMC_NBL0
	PG15   ------> FMC_SDNCAS
	PD0   ------> FMC_D2
	PI7   ------> FMC_D29
	PI6   ------> FMC_D28
	PI5   ------> FMC_NBL3
	PD1   ------> FMC_D3
	PI3   ------> FMC_D27
	PI2   ------> FMC_D26
	PI9   ------> FMC_D30
	PI4   ------> FMC_NBL2
	PH15   ------> FMC_D23
	PI1   ------> FMC_D25
	PF0   ------> FMC_A0
	PI10   ------> FMC_D31
	PH13   ------> FMC_D21
	PH14   ------> FMC_D22
	PI0   ------> FMC_D24
	PH2   ------> FMC_SDCKE0
	PH3   ------> FMC_SDNE0
	PF2   ------> FMC_A2
	PF1   ------> FMC_A1
	PG8   ------> FMC_SDCLK
	PF3   ------> FMC_A3
	PF4   ------> FMC_A4
	PF5   ------> FMC_A5
	PH12   ------> FMC_D20
	PG5   ------> FMC_BA1
	PG4   ------> FMC_BA0
	PH11   ------> FMC_D19
	PH10   ------> FMC_D18
	PD15   ------> FMC_D1
	PG2   ------> FMC_A12
	PC0   ------> FMC_SDNWE
	PG1   ------> FMC_A11
	PH8   ------> FMC_D16
	PH9   ------> FMC_D17
	PD14   ------> FMC_D0
	PF13   ------> FMC_A7
	PG0   ------> FMC_A10
	PE13   ------> FMC_D10
	PD10   ------> FMC_D15
	PF12   ------> FMC_A6
	PF15   ------> FMC_A9
	PE8   ------> FMC_D5
	PE9   ------> FMC_D6
	PE11   ------> FMC_D8
	PE14   ------> FMC_D11
	PD9   ------> FMC_D14
	PD8   ------> FMC_D13
	PF11   ------> FMC_SDNRAS
	PF14   ------> FMC_A8
	PE7   ------> FMC_D4
	PE10   ------> FMC_D7
	PE12   ------> FMC_D9
	PE15   ------> FMC_D12
	*/
    /* GPIO_InitStruct */
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_0 | GPIO_PIN_13 | GPIO_PIN_8
                          | GPIO_PIN_9 | GPIO_PIN_11 | GPIO_PIN_14 | GPIO_PIN_7
                          | GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_15;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* GPIO_InitStruct */
    GPIO_InitStruct.Pin = GPIO_PIN_15 | GPIO_PIN_8 | GPIO_PIN_5 | GPIO_PIN_4
                          | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    /* GPIO_InitStruct */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_15 | GPIO_PIN_14
                          | GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_8;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* GPIO_InitStruct */
    GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_3
                          | GPIO_PIN_2 | GPIO_PIN_9 | GPIO_PIN_4 | GPIO_PIN_1
                          | GPIO_PIN_10 | GPIO_PIN_0;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

    /* GPIO_InitStruct */
    GPIO_InitStruct.Pin = GPIO_PIN_15 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_2
                          | GPIO_PIN_3 | GPIO_PIN_12 | GPIO_PIN_11 | GPIO_PIN_10
                          | GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    /* GPIO_InitStruct */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_3
                          | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_13 | GPIO_PIN_12
                          | GPIO_PIN_15 | GPIO_PIN_11 | GPIO_PIN_14;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    // Init for any pins that can be configured
    GPIO_TypeDef *port;
    port                      = GPIOH;
    GPIO_InitStruct.Pin       = GPIO_PIN_5;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC; // They all seem to use this
    HAL_GPIO_Init(port, &GPIO_InitStruct);

    // This pin can change between boards (SDNWE)
    //	switch(dsy_sdram.board)
    //	{
    //	case DSY_SYS_BOARD_DAISY:
    //		/* GPIO_InitStruct */
    //		GPIO_InitStruct.Pin = GPIO_PIN_0;
    //		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    //		GPIO_InitStruct.Pull = GPIO_NOPULL;
    //		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    //		GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    //		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    //		break;
    //	case DSY_SYS_BOARD_DAISY_SEED:
    //		/* GPIO_InitStruct */
    //		GPIO_InitStruct.Pin = GPIO_PIN_5;
    //		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    //		GPIO_InitStruct.Pull = GPIO_NOPULL;
    //		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    //		GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    //		HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
    //		break;
    //	case DSY_SYS_BOARD_AUDIO_BB:
    //		/* GPIO_InitStruct */
    //		GPIO_InitStruct.Pin = GPIO_PIN_5;
    //		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    //		GPIO_InitStruct.Pull = GPIO_NOPULL;
    //		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    //		GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    //		HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
    //		break;
    //	default:
    //		break;
    //	}

    /* USER CODE BEGIN FMC_MspInit 1 */

    /* USER CODE END FMC_MspInit 1 */
}

void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *sdramHandle)
{
    /* USER CODE BEGIN SDRAM_MspInit 0 */

    /* USER CODE END SDRAM_MspInit 0 */
    HAL_FMC_MspInit();
    /* USER CODE BEGIN SDRAM_MspInit 1 */

    /* USER CODE END SDRAM_MspInit 1 */
}

static uint32_t FMC_DeInitialized = 0;

static void HAL_FMC_MspDeInit(void)
{
    /* USER CODE BEGIN FMC_MspDeInit 0 */

    /* USER CODE END FMC_MspDeInit 0 */
    if(FMC_DeInitialized)
    {
        return;
    }
    FMC_DeInitialized = 1;
    /* Peripheral clock enable */
    __HAL_RCC_FMC_CLK_DISABLE();

    /** FMC GPIO Configuration  
	PE1   ------> FMC_NBL1
	PE0   ------> FMC_NBL0
	PG15   ------> FMC_SDNCAS
	PD0   ------> FMC_D2
	PI7   ------> FMC_D29
	PI6   ------> FMC_D28
	PI5   ------> FMC_NBL3
	PD1   ------> FMC_D3
	PI3   ------> FMC_D27
	PI2   ------> FMC_D26
	PI9   ------> FMC_D30
	PI4   ------> FMC_NBL2
	PH15   ------> FMC_D23
	PI1   ------> FMC_D25
	PF0   ------> FMC_A0
	PI10   ------> FMC_D31
	PH13   ------> FMC_D21
	PH14   ------> FMC_D22
	PI0   ------> FMC_D24
	PH2   ------> FMC_SDCKE0
	PH3   ------> FMC_SDNE0
	PF2   ------> FMC_A2
	PF1   ------> FMC_A1
	PG8   ------> FMC_SDCLK
	PF3   ------> FMC_A3
	PF4   ------> FMC_A4
	PF5   ------> FMC_A5
	PH12   ------> FMC_D20
	PG5   ------> FMC_BA1
	PG4   ------> FMC_BA0
	PH11   ------> FMC_D19
	PH10   ------> FMC_D18
	PD15   ------> FMC_D1
	PG2   ------> FMC_A12
	PC0   ------> FMC_SDNWE
	PG1   ------> FMC_A11
	PH8   ------> FMC_D16
	PH9   ------> FMC_D17
	PD14   ------> FMC_D0
	PF13   ------> FMC_A7
	PG0   ------> FMC_A10
	PE13   ------> FMC_D10
	PD10   ------> FMC_D15
	PF12   ------> FMC_A6
	PF15   ------> FMC_A9
	PE8   ------> FMC_D5
	PE9   ------> FMC_D6
	PE11   ------> FMC_D8
	PE14   ------> FMC_D11
	PD9   ------> FMC_D14
	PD8   ------> FMC_D13
	PF11   ------> FMC_SDNRAS
	PF14   ------> FMC_A8
	PE7   ------> FMC_D4
	PE10   ------> FMC_D7
	PE12   ------> FMC_D9
	PE15   ------> FMC_D12
	*/

    HAL_GPIO_DeInit(GPIOE,
                    GPIO_PIN_1 | GPIO_PIN_0 | GPIO_PIN_13 | GPIO_PIN_8
                        | GPIO_PIN_9 | GPIO_PIN_11 | GPIO_PIN_14 | GPIO_PIN_7
                        | GPIO_PIN_10 | GPIO_PIN_12 | GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOG,
                    GPIO_PIN_15 | GPIO_PIN_8 | GPIO_PIN_5 | GPIO_PIN_4
                        | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0);

    HAL_GPIO_DeInit(GPIOD,
                    GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_15 | GPIO_PIN_14
                        | GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_8);

    HAL_GPIO_DeInit(GPIOI,
                    GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_3
                        | GPIO_PIN_2 | GPIO_PIN_9 | GPIO_PIN_4 | GPIO_PIN_1
                        | GPIO_PIN_10 | GPIO_PIN_0);

    HAL_GPIO_DeInit(GPIOH,
                    GPIO_PIN_15 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_2
                        | GPIO_PIN_3 | GPIO_PIN_12 | GPIO_PIN_11 | GPIO_PIN_10
                        | GPIO_PIN_8 | GPIO_PIN_9);

    HAL_GPIO_DeInit(GPIOF,
                    GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_3
                        | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_13 | GPIO_PIN_12
                        | GPIO_PIN_15 | GPIO_PIN_11 | GPIO_PIN_14);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0);

    /* USER CODE BEGIN FMC_MspDeInit 1 */

    /* USER CODE END FMC_MspDeInit 1 */
}

void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef *sdramHandle)
{
    /* USER CODE BEGIN SDRAM_MspDeInit 0 */

    /* USER CODE END SDRAM_MspDeInit 0 */
    HAL_FMC_MspDeInit();
    /* USER CODE BEGIN SDRAM_MspDeInit 1 */

    /* USER CODE END SDRAM_MspDeInit 1 */
}

void SdramMpuInit()
{
    MPU_Region_InitTypeDef MPU_InitStruct;
    HAL_MPU_Disable();
    // Configure RAM D2 (SRAM1) as non cacheable
    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress      = 0x30000000;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_32KB;
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

    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void __attribute__((constructor)) SDRAM_Init()
{
    //	extern void *_sisdram_data, *_ssdram_data, *_esdram_data;
    //	extern void *_ssdram_bss, *_esdram_bss;

    //	void **pSource, **pDest;
    //	for (pSource = &_sisdram_data, pDest = &_ssdram_data; pDest != &_esdram_data; pSource++, pDest++)
    //		*pDest = *pSource;
    //
    //	for (pDest = &_ssdram_bss; pDest != &_esdram_bss; pDest++)
    //		*pDest = 0;
}