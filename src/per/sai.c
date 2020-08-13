#include "per/sai.h"
#include "daisy_core.h"
#include "sys/dma.h"
#include "util/hal_map.h"

// TODO:
// There is a lot of redundant duplication between sai1 and sai2,
//

SAI_HandleTypeDef hsai_BlockA1;
SAI_HandleTypeDef hsai_BlockB1;
SAI_HandleTypeDef hsai_BlockA2;
SAI_HandleTypeDef hsai_BlockB2;
DMA_HandleTypeDef hdma_sai1_a;
DMA_HandleTypeDef hdma_sai1_b;

DMA_HandleTypeDef hdma_sai2_a;
DMA_HandleTypeDef hdma_sai2_b;

static dsy_sai_handle sai_handle;

//static void dsy_sai_blocks_init(SAI_HandleTypeDef* blockA, SAI_HandleTypeDef* blockB);
static void dsy_sai_blocks_init(uint8_t            sai_idx,
                                SAI_HandleTypeDef* blockA,
                                SAI_HandleTypeDef* blockB);
static void dsy_sai1_init();
static void dsy_sai2_init();

void dsy_sai_init(dsy_audio_sai        init,
                  dsy_audio_samplerate sr[2],
                  dsy_audio_bitdepth   bitdepth[2],
                  dsy_audio_sync       sync_config[2],
                  dsy_gpio_pin*        sai1_pin_list,
                  dsy_gpio_pin*        sai2_pin_list)
{
    for(uint16_t i = 0; i < DSY_SAI_PIN_LAST; i++)
    {
        sai_handle.sai1_pin_config[i] = sai1_pin_list[i];
        sai_handle.sai2_pin_config[i] = sai2_pin_list[i];
    }
    for(uint8_t i = 0; i < 2; i++)
    {
        sai_handle.samplerate[i]  = sr[i];
        sai_handle.bitdepth[i]    = bitdepth[i];
        sai_handle.sync_config[i] = sync_config[i];
    }
    sai_handle.init = init;
    switch(sai_handle.init)
    {
        case DSY_AUDIO_INIT_SAI1: dsy_sai1_init(); break;
        case DSY_AUDIO_INIT_SAI2: dsy_sai2_init(); break;
        case DSY_AUDIO_INIT_BOTH:
            dsy_sai1_init();
            dsy_sai2_init();
            break;
        case DSY_AUDIO_INIT_NONE: break;
        default: break;
    }
}
void dsy_sai_init_from_handle(dsy_sai_handle* hsai)
{
    sai_handle = *hsai;

    // Set Instance for all Blocks
    hsai_BlockA1.Instance = SAI1_Block_A;
    hsai_BlockB1.Instance = SAI1_Block_B;
    hsai_BlockA2.Instance = SAI2_Block_A;
    hsai_BlockB2.Instance = SAI2_Block_B;
    switch(sai_handle.init)
    {
        case DSY_AUDIO_INIT_SAI1:
            dsy_sai_blocks_init(DSY_SAI_1, &hsai_BlockA1, &hsai_BlockB1);
            break;
        case DSY_AUDIO_INIT_SAI2:
            //            dsy_sai_blocks_init(DSY_SAI_2, &hsai_BlockA2, &hsai_BlockB2);
            dsy_sai2_init();
            break;
        case DSY_AUDIO_INIT_BOTH:
            dsy_sai_blocks_init(DSY_SAI_1, &hsai_BlockA1, &hsai_BlockB1);
            //            dsy_sai_blocks_init(DSY_SAI_2, &hsai_BlockA2, &hsai_BlockB2);
            dsy_sai2_init();
            break;
        case DSY_AUDIO_INIT_NONE: break;
        default: break;
    }

    //    switch(sai_handle.init)
    //    {
    //        case DSY_AUDIO_INIT_SAI1: dsy_sai1_init(); break;
    //        case DSY_AUDIO_INIT_SAI2: dsy_sai2_init(); break;
    //        case DSY_AUDIO_INIT_BOTH:
    //            dsy_sai1_init();
    //            dsy_sai2_init();
    //            break;
    //        case DSY_AUDIO_INIT_NONE: break;
    //        default: break;
    //    }
}

static void dsy_sai_blocks_init(uint8_t            sai_idx,
                                SAI_HandleTypeDef* blockA,
                                SAI_HandleTypeDef* blockB)
{
    // Block A
    uint8_t  bd;
    uint32_t protocol;
    switch(sai_handle.bitdepth[sai_idx])
    {
        case DSY_AUDIO_BITDEPTH_16:
            bd       = SAI_PROTOCOL_DATASIZE_16BIT;
            protocol = SAI_I2S_STANDARD;
            break;
        case DSY_AUDIO_BITDEPTH_24:
            bd       = SAI_PROTOCOL_DATASIZE_24BIT;
            protocol = SAI_I2S_MSBJUSTIFIED;
            break;
        default:
            bd       = SAI_PROTOCOL_DATASIZE_16BIT;
            protocol = SAI_I2S_STANDARD;
            break;
    }
    switch(sai_handle.sync_config[sai_idx])
    {
        case DSY_AUDIO_SYNC_SLAVE:
            blockA->Init.AudioMode
                = sai_handle.a_direction[sai_idx] == DSY_AUDIO_RX
                      ? SAI_MODESLAVE_RX
                      : SAI_MODESLAVE_TX;
            break;
        case DSY_AUDIO_SYNC_MASTER:
            blockA->Init.AudioMode
                = sai_handle.a_direction[sai_idx] == DSY_AUDIO_RX
                      ? SAI_MODEMASTER_RX
                      : SAI_MODEMASTER_TX;
            break;
        default: hsai_BlockA1.Init.AudioMode = SAI_MODEMASTER_RX; break;
    }

    blockA->Init.Synchro = SAI_ASYNCHRONOUS;

    blockA->Init.OutputDrive    = SAI_OUTPUTDRIVE_DISABLE;
    blockA->Init.NoDivider      = SAI_MASTERDIVIDER_ENABLE;
    blockA->Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_EMPTY;
    blockA->Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;
    blockA->Init.SynchroExt     = SAI_SYNCEXT_DISABLE;
    blockA->Init.MonoStereoMode = SAI_STEREOMODE;
    blockA->Init.CompandingMode = SAI_NOCOMPANDING;
    if(HAL_SAI_InitProtocol(blockA, protocol, bd, 2) != HAL_OK)
    {
        //Error_Handler();
    }

    blockB->Init.AudioMode = sai_handle.b_direction[sai_idx] == DSY_AUDIO_RX
                                 ? SAI_MODESLAVE_RX
                                 : SAI_MODESLAVE_TX;
    //	hsai_BlockB1.Init.AudioMode		 = SAI_MODESLAVE_TX;
    //    hsai_BlockB1.Init.AudioMode      = SAI_MODESLAVE_RX;
    blockB->Init.Synchro        = SAI_SYNCHRONOUS;
    blockB->Init.OutputDrive    = SAI_OUTPUTDRIVE_DISABLE;
    blockB->Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_EMPTY;
    blockB->Init.SynchroExt     = SAI_SYNCEXT_DISABLE;
    blockB->Init.MonoStereoMode = SAI_STEREOMODE;
    blockB->Init.CompandingMode = SAI_NOCOMPANDING;
    blockB->Init.TriState       = SAI_OUTPUT_NOTRELEASED;

    if(HAL_SAI_InitProtocol(blockB, protocol, bd, 2) != HAL_OK)
    {
        //Error_Handler();
    }
}


/* SAI1 init function */
static void dsy_sai1_init()
{
    uint8_t  bd;
    uint32_t protocol;
    switch(sai_handle.bitdepth[DSY_SAI_1])
    {
        case DSY_AUDIO_BITDEPTH_16:
            bd       = SAI_PROTOCOL_DATASIZE_16BIT;
            protocol = SAI_I2S_STANDARD;
            break;
        case DSY_AUDIO_BITDEPTH_24:
            bd       = SAI_PROTOCOL_DATASIZE_24BIT;
            protocol = SAI_I2S_MSBJUSTIFIED;
            break;
        default:
            bd       = SAI_PROTOCOL_DATASIZE_16BIT;
            protocol = SAI_I2S_STANDARD;
            break;
    }
    switch(sai_handle.sync_config[DSY_SAI_1])
    {
        case DSY_AUDIO_SYNC_SLAVE:
            hsai_BlockA1.Init.AudioMode
                = sai_handle.a_direction[DSY_SAI_1] == DSY_AUDIO_RX
                      ? SAI_MODESLAVE_RX
                      : SAI_MODESLAVE_TX;
            break;
        case DSY_AUDIO_SYNC_MASTER:
            hsai_BlockA1.Init.AudioMode
                = sai_handle.a_direction[DSY_SAI_1] == DSY_AUDIO_RX
                      ? SAI_MODEMASTER_RX
                      : SAI_MODEMASTER_TX;
            break;
        default: hsai_BlockA1.Init.AudioMode = SAI_MODEMASTER_RX; break;
    }
    hsai_BlockA1.Instance            = SAI1_Block_A;
    hsai_BlockA1.Init.Synchro        = SAI_ASYNCHRONOUS;
    hsai_BlockA1.Init.OutputDrive    = SAI_OUTPUTDRIVE_DISABLE;
    hsai_BlockA1.Init.NoDivider      = SAI_MASTERDIVIDER_ENABLE;
    hsai_BlockA1.Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_EMPTY;
    hsai_BlockA1.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;
    hsai_BlockA1.Init.SynchroExt     = SAI_SYNCEXT_DISABLE;
    hsai_BlockA1.Init.MonoStereoMode = SAI_STEREOMODE;
    hsai_BlockA1.Init.CompandingMode = SAI_NOCOMPANDING;
    if(HAL_SAI_InitProtocol(&hsai_BlockA1, protocol, bd, 2) != HAL_OK)
    {
        //Error_Handler();
    }

    hsai_BlockB1.Instance = SAI1_Block_B;
    hsai_BlockB1.Init.AudioMode
        = sai_handle.b_direction[DSY_SAI_1] == DSY_AUDIO_RX ? SAI_MODESLAVE_RX
                                                            : SAI_MODESLAVE_TX;
    //	hsai_BlockB1.Init.AudioMode		 = SAI_MODESLAVE_TX;
    //    hsai_BlockB1.Init.AudioMode      = SAI_MODESLAVE_RX;
    hsai_BlockB1.Init.Synchro        = SAI_SYNCHRONOUS;
    hsai_BlockB1.Init.OutputDrive    = SAI_OUTPUTDRIVE_DISABLE;
    hsai_BlockB1.Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_EMPTY;
    hsai_BlockB1.Init.SynchroExt     = SAI_SYNCEXT_DISABLE;
    hsai_BlockB1.Init.MonoStereoMode = SAI_STEREOMODE;
    hsai_BlockB1.Init.CompandingMode = SAI_NOCOMPANDING;
    hsai_BlockB1.Init.TriState       = SAI_OUTPUT_NOTRELEASED;

    if(HAL_SAI_InitProtocol(&hsai_BlockB1, protocol, bd, 2) != HAL_OK)
    {
        //Error_Handler();
    }
}
/* SAI2 init function */
static void dsy_sai2_init()
{
    uint8_t  bd;
    uint32_t protocol;
    switch(sai_handle.bitdepth[DSY_SAI_2])
    {
        case DSY_AUDIO_BITDEPTH_16:
            bd       = SAI_PROTOCOL_DATASIZE_16BIT;
            protocol = SAI_I2S_STANDARD;
            break;
        case DSY_AUDIO_BITDEPTH_24:
            bd       = SAI_PROTOCOL_DATASIZE_24BIT;
            protocol = SAI_I2S_MSBJUSTIFIED;
            break;
        default:
            bd       = SAI_PROTOCOL_DATASIZE_16BIT;
            protocol = SAI_I2S_STANDARD;
            break;
    }
    hsai_BlockA2.Init.AudioMode      = SAI_MODESLAVE_TX;
    hsai_BlockA2.Instance            = SAI2_Block_A;
    hsai_BlockA2.Init.Synchro        = SAI_SYNCHRONOUS;
    hsai_BlockA2.Init.OutputDrive    = SAI_OUTPUTDRIVE_DISABLE;
    hsai_BlockA2.Init.NoDivider      = SAI_MASTERDIVIDER_ENABLE;
    hsai_BlockA2.Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_EMPTY;
    hsai_BlockA2.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;
    hsai_BlockA2.Init.SynchroExt     = SAI_SYNCEXT_DISABLE;
    hsai_BlockA2.Init.MonoStereoMode = SAI_STEREOMODE;
    hsai_BlockA2.Init.CompandingMode = SAI_NOCOMPANDING;
    if(HAL_SAI_InitProtocol(&hsai_BlockA2, protocol, bd, 2) != HAL_OK)
    {
        //Error_Handler();
    }

    hsai_BlockB2.Instance            = SAI2_Block_B;
    hsai_BlockB2.Init.AudioMode      = SAI_MODEMASTER_RX;
    hsai_BlockB2.Init.Synchro        = SAI_ASYNCHRONOUS;
    hsai_BlockB2.Init.OutputDrive    = SAI_OUTPUTDRIVE_DISABLE;
    hsai_BlockB2.Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_EMPTY;
    hsai_BlockB2.Init.SynchroExt     = SAI_SYNCEXT_DISABLE;
    hsai_BlockB2.Init.MonoStereoMode = SAI_STEREOMODE;
    hsai_BlockB2.Init.CompandingMode = SAI_NOCOMPANDING;
    hsai_BlockB2.Init.TriState       = SAI_OUTPUT_NOTRELEASED;

    if(HAL_SAI_InitProtocol(&hsai_BlockB2, protocol, bd, 2) != HAL_OK)
    {
        //Error_Handler();
    }
}
static uint32_t SAI1_client = 0;
static uint32_t SAI2_client = 0;

// Alternate Function Notes:
// - All SAI1 for all boards so far use AF6
// - All SAI2 for all boards except for PA2 on SeedRev2+ use AF10, PA2 uses AF8
void HAL_SAI_MspInit(SAI_HandleTypeDef* hsai)
{
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct;
    /* SAI1 */
    if(hsai->Instance == SAI1_Block_A)
    {
        /* SAI1 clock enable */
        if(SAI1_client == 0)
        {
            __HAL_RCC_SAI1_CLK_ENABLE();
        }
        SAI1_client++;
        for(uint8_t i = 0; i < DSY_SAI_PIN_LAST; i++)
        {
            GPIO_TypeDef* port;
            if(i != DSY_SAI_PIN_MCLK
               || sai_handle.sync_config[DSY_SAI_1] == DSY_AUDIO_SYNC_MASTER)
            {
                dsy_gpio_pin* p;
                p                     = &sai_handle.sai1_pin_config[i];
                port                  = dsy_hal_map_get_port(p);
                GPIO_InitStruct.Pin   = dsy_hal_map_get_pin(p);
                GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
                GPIO_InitStruct.Pull  = GPIO_NOPULL;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
                GPIO_InitStruct.Alternate
                    = GPIO_AF6_SAI1; // All SAI1 Pins so far use this AF.
                HAL_GPIO_Init(port, &GPIO_InitStruct);
            }
        }

        if(hsai->Init.AudioMode == SAI_MODESLAVE_RX
           || hsai->Init.AudioMode == SAI_MODEMASTER_RX)
        {
            hdma_sai1_a.Init.Direction = DMA_PERIPH_TO_MEMORY;
        }
        else
        {
            hdma_sai1_a.Init.Direction = DMA_MEMORY_TO_PERIPH;
        }
        hdma_sai1_a.Instance                 = DMA1_Stream0;
        hdma_sai1_a.Init.Request             = DMA_REQUEST_SAI1_A;
        hdma_sai1_a.Init.PeriphInc           = DMA_PINC_DISABLE;
        hdma_sai1_a.Init.MemInc              = DMA_MINC_ENABLE;
        hdma_sai1_a.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        hdma_sai1_a.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
        hdma_sai1_a.Init.Mode                = DMA_CIRCULAR;
        hdma_sai1_a.Init.Priority            = DMA_PRIORITY_HIGH;
        hdma_sai1_a.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
        if(HAL_DMA_Init(&hdma_sai1_a) != HAL_OK)
        {
            //Error_Handler();
        }

        /* Several peripheral DMA handle pointers point to the same DMA handle.
     Be aware that there is only one channel to perform all the requested DMAs. */
        __HAL_LINKDMA(hsai, hdmarx, hdma_sai1_a);
        __HAL_LINKDMA(hsai, hdmatx, hdma_sai1_a);
    }
    if(hsai->Instance == SAI1_Block_B)
    {
        /* SAI1 clock enable */
        if(SAI1_client == 0)
        {
            __HAL_RCC_SAI1_CLK_ENABLE();
        }
        SAI1_client++;

        /* Peripheral DMA init*/

        if(hsai->Init.AudioMode == SAI_MODESLAVE_RX
           || hsai->Init.AudioMode == SAI_MODEMASTER_RX)
        {
            hdma_sai1_b.Init.Direction = DMA_PERIPH_TO_MEMORY;
        }
        else
        {
            hdma_sai1_b.Init.Direction = DMA_MEMORY_TO_PERIPH;
        }

        hdma_sai1_b.Instance                 = DMA1_Stream1;
        hdma_sai1_b.Init.Request             = DMA_REQUEST_SAI1_B;
        hdma_sai1_b.Init.PeriphInc           = DMA_PINC_DISABLE;
        hdma_sai1_b.Init.MemInc              = DMA_MINC_ENABLE;
        hdma_sai1_b.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        hdma_sai1_b.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
        hdma_sai1_b.Init.Mode                = DMA_CIRCULAR;
        hdma_sai1_b.Init.Priority            = DMA_PRIORITY_HIGH;
        hdma_sai1_b.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
        if(HAL_DMA_Init(&hdma_sai1_b) != HAL_OK)
        {
            //Error_Handler();
        }

        /* Several peripheral DMA handle pointers point to the same DMA handle.
     Be aware that there is only one channel to perform all the requested DMAs. */
        __HAL_LINKDMA(hsai, hdmarx, hdma_sai1_b);
        __HAL_LINKDMA(hsai, hdmatx, hdma_sai1_b);
    }
    /* SAI2 */
    if(hsai->Instance == SAI2_Block_A)
    {
        /* SAI2 clock enable */
        if(SAI2_client == 0)
        {
            __HAL_RCC_SAI2_CLK_ENABLE();
        }
        SAI2_client++;

        if(hsai->Init.AudioMode == SAI_MODESLAVE_RX
           || hsai->Init.AudioMode == SAI_MODEMASTER_RX)
        {
            hdma_sai2_a.Init.Direction = DMA_PERIPH_TO_MEMORY;
        }
        else
        {
            hdma_sai2_a.Init.Direction = DMA_MEMORY_TO_PERIPH;
        }
        hdma_sai2_a.Instance                 = DMA1_Stream3;
        hdma_sai2_a.Init.Request             = DMA_REQUEST_SAI2_A;
        hdma_sai2_a.Init.PeriphInc           = DMA_PINC_DISABLE;
        hdma_sai2_a.Init.MemInc              = DMA_MINC_ENABLE;
        hdma_sai2_a.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        hdma_sai2_a.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
        hdma_sai2_a.Init.Mode                = DMA_CIRCULAR;
        hdma_sai2_a.Init.Priority            = DMA_PRIORITY_HIGH;
        hdma_sai2_a.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
        if(HAL_DMA_Init(&hdma_sai2_a) != HAL_OK)
        {
            //Error_Handler();
        }
        //        pSyncConfig.SyncSignalID  = HAL_DMAMUX1_SYNC_DMAMUX1_CH1_EVT;
        //        pSyncConfig.SyncPolarity  = HAL_DMAMUX_SYNC_RISING;
        //        pSyncConfig.SyncEnable    = ENABLE;
        //        pSyncConfig.EventEnable   = DISABLE;
        //        pSyncConfig.RequestNumber = 1;
        //        if(HAL_DMAEx_ConfigMuxSync(&hdma_sai2_a, &pSyncConfig) != HAL_OK)
        //        {
        //            //Error_Handler();
        //        }

        /* Several peripheral DMA handle pointers point to the same DMA handle.
     Be aware that there is only one channel to perform all the requested DMAs. */
        __HAL_LINKDMA(hsai, hdmarx, hdma_sai2_a);
        __HAL_LINKDMA(hsai, hdmatx, hdma_sai2_a);
    }
    if(hsai->Instance == SAI2_Block_B)
    {
        /* SAI2 clock enable */
        if(SAI2_client == 0)
        {
            __HAL_RCC_SAI2_CLK_ENABLE();
        }
        SAI2_client++;

        // New style config:
        for(uint8_t i = 0; i < DSY_SAI_PIN_LAST; i++)
        {
            GPIO_TypeDef* port;
            if(i != DSY_SAI_PIN_MCLK
               || sai_handle.sync_config[DSY_SAI_2] == DSY_AUDIO_SYNC_MASTER)
            {
                dsy_gpio_pin* p;
                p                     = &sai_handle.sai2_pin_config[i];
                port                  = dsy_hal_map_get_port(p);
                GPIO_InitStruct.Pin   = dsy_hal_map_get_pin(p);
                GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
                GPIO_InitStruct.Pull  = GPIO_NOPULL;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
                GPIO_InitStruct.Alternate
                    = i == DSY_SAI_PIN_SCK ? GPIO_AF8_SAI2 : GPIO_AF10_SAI2;
                HAL_GPIO_Init(port, &GPIO_InitStruct);
            }
        }

        if(hsai->Init.AudioMode == SAI_MODESLAVE_RX
           || hsai->Init.AudioMode == SAI_MODEMASTER_RX)
        {
            hdma_sai2_b.Init.Direction = DMA_PERIPH_TO_MEMORY;
        }
        else
        {
            hdma_sai2_b.Init.Direction = DMA_MEMORY_TO_PERIPH;
        }

        hdma_sai2_b.Instance                 = DMA1_Stream4;
        hdma_sai2_b.Init.Request             = DMA_REQUEST_SAI2_B;
        hdma_sai2_b.Init.PeriphInc           = DMA_PINC_DISABLE;
        hdma_sai2_b.Init.MemInc              = DMA_MINC_ENABLE;
        hdma_sai2_b.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
        hdma_sai2_b.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
        hdma_sai2_b.Init.Mode                = DMA_CIRCULAR;
        hdma_sai2_b.Init.Priority            = DMA_PRIORITY_HIGH;
        hdma_sai2_b.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
        if(HAL_DMA_Init(&hdma_sai2_b) != HAL_OK)
        {
            //Error_Handler();
        }

        //        pSyncConfig.SyncSignalID  = HAL_DMAMUX1_SYNC_DMAMUX1_CH1_EVT;
        //        pSyncConfig.SyncPolarity  = HAL_DMAMUX_SYNC_RISING;
        //        pSyncConfig.SyncEnable    = ENABLE;
        //        pSyncConfig.EventEnable   = DISABLE;
        //        pSyncConfig.RequestNumber = 1;
        //        if(HAL_DMAEx_ConfigMuxSync(&hdma_sai2_b, &pSyncConfig) != HAL_OK)
        //        {
        //            //Error_Handler();
        //        }
        /* Several peripheral DMA handle pointers point to the same DMA handle.
     Be aware that there is only one channel to perform all the requested DMAs. */
        __HAL_LINKDMA(hsai, hdmarx, hdma_sai2_b);
        __HAL_LINKDMA(hsai, hdmatx, hdma_sai2_b);
    }
}

void HAL_SAI_MspDeInit(SAI_HandleTypeDef* hsai)
{
    /* SAI1 */
    if(hsai->Instance == SAI1_Block_A)
    {
        SAI1_client--;
        if(SAI1_client == 0)
        {
            __HAL_RCC_SAI1_CLK_DISABLE();
        }
        for(uint8_t i = 0; i < DSY_SAI_PIN_LAST; i++)
        {
            GPIO_TypeDef* port;
            uint16_t      pin;
            if(i != DSY_SAI_PIN_MCLK
               || sai_handle.sync_config[DSY_SAI_1] == DSY_AUDIO_SYNC_MASTER)
            {
                dsy_gpio_pin* p;
                p    = &sai_handle.sai1_pin_config[i];
                port = dsy_hal_map_get_port(p);
                pin  = dsy_hal_map_get_pin(p);
                HAL_GPIO_DeInit(port, pin);
            }
        }
        HAL_DMA_DeInit(hsai->hdmarx);
        HAL_DMA_DeInit(hsai->hdmatx);
    }
    if(hsai->Instance == SAI1_Block_B)
    {
        SAI1_client--;
        if(SAI1_client == 0)
        {
            __HAL_RCC_SAI1_CLK_DISABLE();
        }
        for(uint8_t i = 0; i < DSY_SAI_PIN_LAST; i++)
        {
            GPIO_TypeDef* port;
            uint16_t      pin;
            if(i != DSY_SAI_PIN_MCLK
               || sai_handle.sync_config[DSY_SAI_1] == DSY_AUDIO_SYNC_MASTER)
            {
                dsy_gpio_pin* p;
                p    = &sai_handle.sai1_pin_config[i];
                port = dsy_hal_map_get_port(p);
                pin  = dsy_hal_map_get_pin(p);
                HAL_GPIO_DeInit(port, pin);
            }
        }

        HAL_DMA_DeInit(hsai->hdmarx);
        HAL_DMA_DeInit(hsai->hdmatx);
    }
    /* SAI2 */
    if(hsai->Instance == SAI2_Block_A)
    {
        SAI2_client--;
        if(SAI2_client == 0)
        {
            __HAL_RCC_SAI2_CLK_DISABLE();
        }
        for(uint8_t i = 0; i < DSY_SAI_PIN_LAST; i++)
        {
            GPIO_TypeDef* port;
            uint16_t      pin;
            if(i != DSY_SAI_PIN_MCLK
               || sai_handle.sync_config[DSY_SAI_2] == DSY_AUDIO_SYNC_MASTER)
            {
                dsy_gpio_pin* p;
                p    = &sai_handle.sai2_pin_config[i];
                port = dsy_hal_map_get_port(p);
                pin  = dsy_hal_map_get_pin(p);
                //				port = (GPIO_TypeDef*)
                //					gpio_hal_port_map[sai_handle.sai2_pin_config[i].port];
                //				pin
                //					= gpio_hal_pin_map[sai_handle.sai2_pin_config[i].pin];
                HAL_GPIO_DeInit(port, pin);
            }
        }
    }
    if(hsai->Instance == SAI2_Block_B)
    {
        SAI2_client--;
        if(SAI2_client == 0)
        {
            __HAL_RCC_SAI2_CLK_DISABLE();
        }
        for(uint8_t i = 0; i < DSY_SAI_PIN_LAST; i++)
        {
            GPIO_TypeDef* port;
            uint16_t      pin;
            if(i != DSY_SAI_PIN_MCLK
               || sai_handle.sync_config[DSY_SAI_2] == DSY_AUDIO_SYNC_MASTER)
            {
                dsy_gpio_pin* p;
                p    = &sai_handle.sai2_pin_config[i];
                port = dsy_hal_map_get_port(p);
                pin  = dsy_hal_map_get_pin(p);
                //				port = (GPIO_TypeDef*)
                //					gpio_hal_port_map[sai_handle.sai2_pin_config[i].port];
                //				pin
                //					= gpio_hal_pin_map[sai_handle.sai2_pin_config[i].pin];
                HAL_GPIO_DeInit(port, pin);
            }
        }
    }
}
