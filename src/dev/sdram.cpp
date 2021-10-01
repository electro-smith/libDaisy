#include <stm32h7xx_hal.h>
#include "dev/sdram.h"
extern "C"
{
#include "util/hal_map.h"
}

// TODO:
// - Consider alternative to libdaisy.h inclusion for board specific details.
// - Optimize Timing Variables, etc. for Maximum Speed.

// For now all configuration is done specifically for
//    the AS4C16M32MSA-6BIN 64MB SDRAM from Alliance Memory.

// Notes from the Datasheet
// tCK(3) - Clock Cycle Time (min.) 6ns
// tAC(3) - Access time from Clk (max.) 5.5ns
// tRAS      - Row Active time (min.) 48ns
// tRC    - Row Cycle time (min.) 60ns

// 166MHz = 6.024ns
// RAS = 8 ticks at 166
// RC = 10 ticks at 166

//#include "fmc.h"
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

static dsy_sdram_t dsy_sdram;

SdramHandle::Result SdramHandle::Init()
{
    if(PeriphInit() != Result::OK)
    {
        return Result::ERR;
    }
    if(DeviceInit() != Result::OK)
    {
        return Result::ERR;
    }
    return Result::OK;
}

SdramHandle::Result SdramHandle::DeInit()
{
    if(DeviceDeInit() != Result::OK)
    {
        return Result::ERR;
    }
    if(PeriphDeInit() != Result::OK)
    {
        return Result::ERR;
    }
    return Result::OK;
}

SdramHandle::Result SdramHandle::PeriphInit()
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
    dsy_sdram.hsdram.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    dsy_sdram.hsdram.Init.SDClockPeriod   = FMC_SDRAM_CLOCK_PERIOD_2;
    dsy_sdram.hsdram.Init.ReadBurst       = FMC_SDRAM_RBURST_ENABLE;
    dsy_sdram.hsdram.Init.ReadPipeDelay   = FMC_SDRAM_RPIPE_DELAY_0;
    /* SdramTiming */
    SdramTiming.LoadToActiveDelay    = 2;
    SdramTiming.ExitSelfRefreshDelay = 7;
    SdramTiming.SelfRefreshTime      = 4;
    SdramTiming.RowCycleDelay        = 8; // started at 7
    SdramTiming.WriteRecoveryTime    = 3;
    SdramTiming.RPDelay              = 16;
    SdramTiming.RCDDelay             = 10; // started at 2
    //    SdramTiming.LoadToActiveDelay = 16;
    //    SdramTiming.ExitSelfRefreshDelay = 16;
    //    SdramTiming.SelfRefreshTime = 16;
    //    SdramTiming.RowCycleDelay = 16;
    //    SdramTiming.WriteRecoveryTime = 16;
    //    SdramTiming.RPDelay = 16;
    //    SdramTiming.RCDDelay = 16;

    if(HAL_SDRAM_Init(&dsy_sdram.hsdram, &SdramTiming) != HAL_OK)
    {
        //Error_Handler();
        return Result::ERR;
    }
    return Result::OK;
}

SdramHandle::Result SdramHandle::DeviceInit()
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
    HAL_Delay(100);


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
    return Result::OK;
}

SdramHandle::Result SdramHandle::DeviceDeInit()
{
    // I believe this is sufficient to enter power down mode
    FMC_SDRAM_CommandTypeDef Command;
    /* Send the module into powerdown mode */
    Command.CommandMode            = FMC_SDRAM_CMD_POWERDOWN_MODE;
    Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    HAL_SDRAM_SendCommand(&dsy_sdram.hsdram, &Command, 0x1000);
    return Result::OK;
}

SdramHandle::Result SdramHandle::PeriphDeInit()
{
    if(HAL_SDRAM_DeInit(&dsy_sdram.hsdram) != HAL_OK)
        return Result::ERR;

    return Result::OK;
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

//Program the <project name>_QSPIFLASH.bin file to QSPIFLASH manually
