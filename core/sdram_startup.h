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

// typedef struct
// {
//   uint8_t             board;
//   SDRAM_HandleTypeDef hsdram;
// } dsy_sdram_t;

void SdramInit();
void PeriphInit();
void DeviceInit();
void SendCommand(uint32_t mode, uint32_t target, uint32_t refresh, uint32_t definition);
void SdramMpuInit();

static void HAL_SDRAM_GpioInit();
static void HAL_FMC_GpioInit();
static void HAL_SDRAM_GpioDeInit();
static void HAL_FMC_GpioDeInit();

void SdramInit()
{
  PeriphInit();
  DeviceInit();
  SdramMpuInit();
}

// static dsy_sdram_t dsy_sdram;

/* --- SDCR Register ---*/
/* SDCR register clear mask */
#define SDCR_CLEAR_MASK   ((uint32_t)(FMC_SDCRx_NC    | FMC_SDCRx_NR     | \
                                      FMC_SDCRx_MWID  | FMC_SDCRx_NB     | \
                                      FMC_SDCRx_CAS   | FMC_SDCRx_WP     | \
                                      FMC_SDCRx_SDCLK | FMC_SDCRx_RBURST | \
                                      FMC_SDCRx_RPIPE))

/* --- SDTR Register ---*/
/* SDTR register clear mask */
#define SDTR_CLEAR_MASK   ((uint32_t)(FMC_SDTRx_TMRD  | FMC_SDTRx_TXSR   | \
                                      FMC_SDTRx_TRAS  | FMC_SDTRx_TRC    | \
                                      FMC_SDTRx_TWR   | FMC_SDTRx_TRP    | \
                                      FMC_SDTRx_TRCD))

void PeriphInit()
{
    // __FMC_DISABLE();

    HAL_SDRAM_GpioInit(NULL); // the function doesn't use it anyway

    // just for the best clarity:
    // Init
    // const uint32_t SDBank             = FMC_SDRAM_BANK1;
    const uint32_t ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_9;
    const uint32_t RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_13;
    const uint32_t MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_32;
    const uint32_t InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    const uint32_t CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
    const uint32_t WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    const uint32_t SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_2;
    const uint32_t ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
    const uint32_t ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;

    MODIFY_REG (
        FMC_SDRAM_DEVICE->SDCR[FMC_SDRAM_BANK1],
        SDCR_CLEAR_MASK,
        (
            ColumnBitsNumber   |
            RowBitsNumber      |
            MemoryDataWidth    |
            InternalBankNumber |
            CASLatency         |
            WriteProtection    |
            SDClockPeriod      |
            ReadBurst          |
            ReadPipeDelay
        )
    );

    const uint32_t LoadToActiveDelay    = 2;
    const uint32_t ExitSelfRefreshDelay = 7;
    const uint32_t SelfRefreshTime      = 4;
    const uint32_t RowCycleDelay        = 8; // started at 7
    const uint32_t WriteRecoveryTime    = 3;
    const uint32_t RPDelay              = 1;
    const uint32_t RCDDelay             = 10; // started at 2


    MODIFY_REG(
        FMC_SDRAM_DEVICE->SDTR[FMC_SDRAM_BANK1],
        SDTR_CLEAR_MASK,
        (
            ((LoadToActiveDelay - 1U)    << 0U)                 |
            ((ExitSelfRefreshDelay - 1U) << FMC_SDTRx_TXSR_Pos) |
            ((SelfRefreshTime - 1U)      << FMC_SDTRx_TRAS_Pos) |
            ((RowCycleDelay - 1U)        << FMC_SDTRx_TRC_Pos)  |
            ((WriteRecoveryTime - 1U)    << FMC_SDTRx_TWR_Pos)  |
            ((RPDelay - 1U)              << FMC_SDTRx_TRP_Pos)  |
            ((RCDDelay - 1U)             << FMC_SDTRx_TRCD_Pos)
        )
    );

    __FMC_ENABLE();
}

void SendCommand(uint32_t mode, uint32_t target, uint32_t refresh, uint32_t definition)
{
    // Shouldn't this really just be writing to the register, not ORing?
    SET_BIT (
        FMC_SDRAM_DEVICE->SDCMR, 
        (
            mode                                   |
            target                                 |
            ((refresh - 1U) << FMC_SDCMR_NRFS_Pos) |
            (definition   << FMC_SDCMR_MRD_Pos)
        )
    );
}

void DeviceInit()
{
    /* Step 3:  Configure a clock configuration enable command */
    SendCommand(FMC_SDRAM_CMD_CLK_ENABLE, FMC_SDRAM_CMD_TARGET_BANK1, 1, 0);

    // TODO -- double check that this takes the appropriate amount of time (seems a bit long).
    // Volatile to (hopefully) ensure no optimization occurs. This may need to be accompanied with
    // pragmas and attributes to be compiler cross-compatible.

    // The startup clock is 64MHz, and this loop
    // typically compiles to 6 instructions, so for
    // a delay of ~10 ms...
    for (volatile int i = 0; i < (int) (64e4 / 6); i++);

    /* Step 5: Configure a PALL (precharge all) command */
    SendCommand(FMC_SDRAM_CMD_PALL, FMC_SDRAM_CMD_TARGET_BANK1, 1, 0);

    /* Step 6 : Configure a Auto-Refresh command */
    SendCommand(FMC_SDRAM_CMD_AUTOREFRESH_MODE, FMC_SDRAM_CMD_TARGET_BANK1, 4, 0);

    /* Step 7: Program the external memory mode register */
    __IO uint32_t tmpmrd =  (uint32_t) SDRAM_MODEREG_BURST_LENGTH_4 | 
                            SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL     | 
                            SDRAM_MODEREG_CAS_LATENCY_3             | 
                            SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
    //SDRAM_MODEREG_OPERATING_MODE_STANDARD | // Used in example, but can't find reference except for "Test Mode"

    SendCommand(FMC_SDRAM_CMD_LOAD_MODE, FMC_SDRAM_CMD_TARGET_BANK1, 1, tmpmrd);
    
    /* Set the refresh rate in command register */
    const uint32_t RefreshRate = 0x81A - 20;
    MODIFY_REG(FMC_SDRAM_DEVICE->SDRTR, FMC_SDRTR_COUNT, (RefreshRate << FMC_SDRTR_COUNT_Pos));
}

static uint32_t FMC_Initialized = 0;

static void HAL_FMC_GpioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

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

    /* USER CODE BEGIN FMC_GpioInit 1 */

    /* USER CODE END FMC_GpioInit 1 */
}

void HAL_SDRAM_GpioInit(SDRAM_HandleTypeDef *sdramHandle)
{
    /* USER CODE BEGIN SDRAM_GpioInit 0 */

    /* USER CODE END SDRAM_GpioInit 0 */
    HAL_FMC_GpioInit();
    /* USER CODE BEGIN SDRAM_GpioInit 1 */

    /* USER CODE END SDRAM_GpioInit 1 */
}

static uint32_t FMC_DeInitialized = 0;

static void HAL_FMC_GpioDeInit(void)
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

void HAL_SDRAM_GpioDeInit(SDRAM_HandleTypeDef *sdramHandle)
{
    /* USER CODE BEGIN SDRAM_MspDeInit 0 */

    /* USER CODE END SDRAM_MspDeInit 0 */
    HAL_FMC_GpioDeInit();
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