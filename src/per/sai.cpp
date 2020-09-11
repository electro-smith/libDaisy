#include "per/sai.h"
#include "daisy_core.h"
#include <string.h>

extern "C"
{
#include "sys/dma.h"
#include "util/hal_map.h"
}

namespace daisy
{
class SaiHandle::Impl
{
  public:
    SaiHandle::Result        Init(const SaiHandle::Config& config);
    const SaiHandle::Config& GetConfig() const { return config_; }

    SaiHandle::Result
                      StartDmaTransfer(int32_t* buffer_rx, int32_t* buffer_tx, size_t size);
    SaiHandle::Result StopDmaTransfer();

    SaiHandle::Config config_;
    SAI_HandleTypeDef sai_a_handle_, sai_b_handle_;
    DMA_HandleTypeDef sai_a_dma_handle_, sai_b_dma_handle_;

    int32_t *buff_rx_, *buff_tx_;
    size_t   buff_size_;

    // If we need it, which I don't think we do for this one.
    //static void GlobalInit();

    void InitPins();
    void DeinitPins();

    void InitDma();
    void DeinitDma();

    void TestCallback(size_t offset);
};

// ================================================================
// Generic Error Handler
// ================================================================

static void Error_Handler()
{
    asm("bkpt 255");
    while(1) {}
}

// ================================================================
// Static References for available SaiHandle::Impls
// ================================================================

static SaiHandle::Impl sai_handles[2];

// ================================================================
// Scheduling and global functions
// ================================================================

// If we need it, which I don't think we do for this one.
//void SaiHandle::Impl::GlobalInit() {}


// ================================================================
// SAI Functions
// ================================================================

SaiHandle::Result SaiHandle::Impl::Init(const SaiHandle::Config& config)
{
    // do some init stuff
    const int sai_idx = int(config.periph);
    if(sai_idx >= 2)
        return Result::ERR;

    // Default Buffer states
    buff_rx_   = nullptr;
    buff_tx_   = nullptr;
    buff_size_ = 0;

    config_                                     = config;
    constexpr SAI_Block_TypeDef* a_instances[2] = {SAI1_Block_A, SAI2_Block_A};
    constexpr SAI_Block_TypeDef* b_instances[2] = {SAI1_Block_B, SAI2_Block_B};

    sai_a_handle_.Instance = a_instances[sai_idx];
    sai_b_handle_.Instance = b_instances[sai_idx];

    // Samplerate
    switch(config.sr)
    {
        case Config::SampleRate::SAI_8KHZ:
            sai_a_handle_.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_8K;
            sai_b_handle_.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_8K;
            break;
        case Config::SampleRate::SAI_16KHZ:
            sai_a_handle_.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_16K;
            sai_b_handle_.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_16K;
            break;
        case Config::SampleRate::SAI_32KHZ:
            sai_a_handle_.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_32K;
            sai_b_handle_.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_32K;
            break;
        case Config::SampleRate::SAI_48KHZ:
            sai_a_handle_.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;
            sai_b_handle_.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;
            break;
        case Config::SampleRate::SAI_96KHZ:
            sai_a_handle_.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_96K;
            sai_b_handle_.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_96K;
            break;
        default: break;
    }
    // Audio Mode A
    if(config.a_sync == Config::Sync::MASTER)
    {
        sai_a_handle_.Init.AudioMode
            = config.a_dir == Config::Direction::TRANSMIT ? SAI_MODEMASTER_TX
                                                          : SAI_MODEMASTER_RX;
    }
    else
    {
        sai_a_handle_.Init.AudioMode
            = config.a_dir == Config::Direction::TRANSMIT ? SAI_MODESLAVE_TX
                                                          : SAI_MODESLAVE_RX;
    }
    // Audio Mode B
    if(config.b_sync == Config::Sync::MASTER)
    {
        sai_b_handle_.Init.AudioMode
            = config.b_dir == Config::Direction::TRANSMIT ? SAI_MODEMASTER_TX
                                                          : SAI_MODEMASTER_RX;
    }
    else
    {
        sai_b_handle_.Init.AudioMode
            = config.b_dir == Config::Direction::TRANSMIT ? SAI_MODESLAVE_TX
                                                          : SAI_MODESLAVE_RX;
    }
    // Bitdepth / protocol (currently based on bitdepth..)
    // TODO probably split these up for better flexibility..
    // These are also currently fixed to be the same per block.
    uint8_t  bd;
    uint32_t protocol;
    switch(config.bit_depth)
    {
        case Config::BitDepth::SAI_16BIT:
            bd       = SAI_PROTOCOL_DATASIZE_16BIT;
            protocol = SAI_I2S_STANDARD;
            break;
        case Config::BitDepth::SAI_24BIT:
            bd       = SAI_PROTOCOL_DATASIZE_16BIT;
            protocol = SAI_I2S_STANDARD;
            break;
        case Config::BitDepth::SAI_32BIT:
            bd       = SAI_PROTOCOL_DATASIZE_16BIT;
            protocol = SAI_I2S_STANDARD;
            break;
        default: break;
    }

    // Generic Inits that we don't have API control over.
    // A
    sai_a_handle_.Init.OutputDrive    = SAI_OUTPUTDRIVE_DISABLE;
    sai_a_handle_.Init.NoDivider      = SAI_MASTERDIVIDER_ENABLE;
    sai_a_handle_.Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_EMPTY;
    sai_a_handle_.Init.MonoStereoMode = SAI_STEREOMODE;
    sai_a_handle_.Init.CompandingMode = SAI_NOCOMPANDING;
    sai_a_handle_.Init.TriState       = SAI_OUTPUT_NOTRELEASED;
    // B
    sai_b_handle_.Init.OutputDrive    = SAI_OUTPUTDRIVE_DISABLE;
    sai_b_handle_.Init.NoDivider      = SAI_MASTERDIVIDER_ENABLE;
    sai_b_handle_.Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_EMPTY;
    sai_b_handle_.Init.MonoStereoMode = SAI_STEREOMODE;
    sai_b_handle_.Init.CompandingMode = SAI_NOCOMPANDING;
    sai_b_handle_.Init.TriState       = SAI_OUTPUT_NOTRELEASED;

    if(HAL_SAI_InitProtocol(&sai_a_handle_, protocol, bd, 2) != HAL_OK)
    {
        Error_Handler();
        return Result::ERR;
    }

    if(HAL_SAI_InitProtocol(&sai_b_handle_, protocol, bd, 2) != HAL_OK)
    {
        Error_Handler();
        return Result::ERR;
    }

    return Result::OK;
}

void SaiHandle::Impl::InitDma()
{
    // DMA Init
    const int                     sai_idx = int(config_.periph);
    uint32_t                      req_a, req_b;
    uint32_t                      dir_a, dir_b;
    constexpr DMA_Stream_TypeDef* a_dma_instances[2]{DMA1_Stream0,
                                                     DMA1_Stream2};
    constexpr DMA_Stream_TypeDef* b_dma_instances[2]{DMA1_Stream1,
                                                     DMA1_Stream3};
    // Set Conditional DMA settings
    req_a = sai_idx == int(Config::Peripheral::SAI_1) ? DMA_REQUEST_SAI1_A
                                                      : DMA_REQUEST_SAI2_A;
    req_b = sai_idx == int(Config::Peripheral::SAI_1) ? DMA_REQUEST_SAI1_B
                                                      : DMA_REQUEST_SAI2_B;
    dir_a = config_.a_dir == Config::Direction::RECEIVE ? DMA_PERIPH_TO_MEMORY
                                                        : DMA_MEMORY_TO_PERIPH;
    dir_b = config_.b_dir == Config::Direction::RECEIVE ? DMA_PERIPH_TO_MEMORY
                                                        : DMA_MEMORY_TO_PERIPH;
    sai_a_dma_handle_.Instance = a_dma_instances[sai_idx];
    sai_b_dma_handle_.Instance = b_dma_instances[sai_idx];

    // Generic
    // A
    sai_a_dma_handle_.Init.Request             = req_a;
    sai_a_dma_handle_.Init.Direction           = dir_a;
    sai_a_dma_handle_.Init.PeriphInc           = DMA_PINC_DISABLE;
    sai_a_dma_handle_.Init.MemInc              = DMA_MINC_ENABLE;
    sai_a_dma_handle_.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    sai_a_dma_handle_.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    sai_a_dma_handle_.Init.Mode                = DMA_CIRCULAR;
    sai_a_dma_handle_.Init.Priority            = DMA_PRIORITY_HIGH;
    sai_a_dma_handle_.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    if(HAL_DMA_Init(&sai_a_dma_handle_) != HAL_OK)
    {
        Error_Handler();
    }
    // B
    sai_b_dma_handle_.Init.Request             = req_b;
    sai_b_dma_handle_.Init.Direction           = dir_b;
    sai_b_dma_handle_.Init.PeriphInc           = DMA_PINC_DISABLE;
    sai_b_dma_handle_.Init.MemInc              = DMA_MINC_ENABLE;
    sai_b_dma_handle_.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    sai_b_dma_handle_.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    sai_b_dma_handle_.Init.Mode                = DMA_CIRCULAR;
    sai_b_dma_handle_.Init.Priority            = DMA_PRIORITY_HIGH;
    sai_b_dma_handle_.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    if(HAL_DMA_Init(&sai_b_dma_handle_) != HAL_OK)
    {
        Error_Handler();
    }
    // Kinda weird because macro logic. . .
    if(config_.a_dir == Config::Direction::RECEIVE)
        __HAL_LINKDMA(&sai_a_handle_, hdmarx, sai_a_dma_handle_);
    else // TRANSMIT
        __HAL_LINKDMA(&sai_a_handle_, hdmatx, sai_a_dma_handle_);

    if(config_.b_dir == Config::Direction::RECEIVE)
        __HAL_LINKDMA(&sai_b_handle_, hdmarx, sai_b_dma_handle_);
    else // TRANSMIT
        __HAL_LINKDMA(&sai_b_handle_, hdmatx, sai_b_dma_handle_);
}

void SaiHandle::Impl::DeinitDma()
{
    HAL_DMA_DeInit(sai_a_handle_.hdmarx);
    HAL_DMA_DeInit(sai_a_handle_.hdmatx);
}

void SaiHandle::Impl::TestCallback(size_t offset)
{
    if(buff_tx_ != nullptr && buff_rx_ != nullptr)
        memcpy(buff_tx_, buff_rx_, sizeof(buff_tx_[0]) * (buff_size_ / 2));
}

SaiHandle::Result SaiHandle::Impl::StartDmaTransfer(int32_t* buffer_rx,
                                                    int32_t* buffer_tx,
                                                    size_t   size)
{
    buff_rx_   = buffer_rx;
    buff_tx_   = buffer_tx;
    buff_size_ = size;
    config_.a_dir == Config::Direction::RECEIVE
        ? HAL_SAI_Receive_DMA(&sai_a_handle_, (uint8_t*)buffer_rx, size)
        : HAL_SAI_Transmit_DMA(&sai_a_handle_, (uint8_t*)buffer_tx, size);
    config_.b_dir == Config::Direction::RECEIVE
        ? HAL_SAI_Receive_DMA(&sai_b_handle_, (uint8_t*)buffer_rx, size)
        : HAL_SAI_Transmit_DMA(&sai_b_handle_, (uint8_t*)buffer_tx, size);
    return Result::OK;
}
SaiHandle::Result SaiHandle::Impl::StopDmaTransfer()
{
    HAL_SAI_DMAStop(&sai_a_handle_);
    HAL_SAI_DMAStop(&sai_b_handle_);
    return Result::OK;
}

void SaiHandle::Impl::InitPins()
{
    bool             is_master;
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_TypeDef*    port;
    dsy_gpio_pin*    pin_cfg;
    dsy_gpio_pin*    cfg[] = {&config_.pin_config.fs,
                           &config_.pin_config.mclk,
                           &config_.pin_config.sck,
                           &config_.pin_config.sa,
                           &config_.pin_config.sb};
    // Special Case checks
    dsy_gpio_pin sck_af_pin = {DSY_GPIOA, 2};
    is_master               = (config_.a_sync == Config::Sync::MASTER
                 || config_.b_sync == Config::Sync::MASTER);
    // Generics
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    for(size_t i = 0; i < 5; i++)
    {
        // Skip MCLK if not master.
        if(dsy_pin_cmp(&config_.pin_config.mclk, cfg[i]) && !is_master)
            continue;

        // Special case for SAI2-sck (should add a map for Alternate Functions at some point..)
        switch(config_.periph)
        {
            case Config::Peripheral::SAI_1:
                GPIO_InitStruct.Alternate = GPIO_AF6_SAI1;
                break;
            case Config::Peripheral::SAI_2:
                GPIO_InitStruct.Alternate = dsy_pin_cmp(pin_cfg, &sck_af_pin)
                                                ? GPIO_AF10_SAI2
                                                : GPIO_AF8_SAI2;
                break;
            default: break;
        }

        GPIO_InitStruct.Pin = dsy_hal_map_get_pin(cfg[i]);
        port                = dsy_hal_map_get_port(cfg[i]);
        HAL_GPIO_Init(port, &GPIO_InitStruct);
    }
}

void SaiHandle::Impl::DeinitPins()
{
    GPIO_TypeDef* port;
    uint16_t      pin;
    bool          is_master;
    is_master = (config_.a_sync == Config::Sync::MASTER
                 || config_.b_sync == Config::Sync::MASTER);
    if(is_master)
    {
        port = dsy_hal_map_get_port(&config_.pin_config.mclk);
        pin  = dsy_hal_map_get_pin(&config_.pin_config.mclk);
        HAL_GPIO_DeInit(port, pin);
    }
    port = dsy_hal_map_get_port(&config_.pin_config.fs);
    pin  = dsy_hal_map_get_pin(&config_.pin_config.fs);
    HAL_GPIO_DeInit(port, pin);
    port = dsy_hal_map_get_port(&config_.pin_config.sck);
    pin  = dsy_hal_map_get_pin(&config_.pin_config.sck);
    HAL_GPIO_DeInit(port, pin);
    port = dsy_hal_map_get_port(&config_.pin_config.sa);
    pin  = dsy_hal_map_get_pin(&config_.pin_config.sa);
    HAL_GPIO_DeInit(port, pin);
    port = dsy_hal_map_get_port(&config_.pin_config.sb);
    pin  = dsy_hal_map_get_pin(&config_.pin_config.sb);
    HAL_GPIO_DeInit(port, pin);
}

// ================================================================
// HAL Service Functions
// ================================================================

extern "C" void HAL_SAI_MspInit(SAI_HandleTypeDef* sai_handle)
{
    // Due to the BlockA/BlockB stuff right now
    // it is required that they both be used.
    // We could add a layer of separate config for them.
    if(sai_handle->Instance == SAI1_Block_A)
    {
        __HAL_RCC_GPIOE_CLK_ENABLE();
        sai_handles[0].InitPins();
        __HAL_RCC_SAI1_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();
        sai_handles[0].InitDma();
        HAL_NVIC_SetPriority(SAI1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(SAI1_IRQn);
    }
    else if(sai_handle->Instance == SAI2_Block_A)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        __HAL_RCC_GPIOG_CLK_ENABLE();
        sai_handles[1].InitPins();
        __HAL_RCC_SAI2_CLK_ENABLE();
        __HAL_RCC_DMA1_CLK_ENABLE();
        sai_handles[1].InitDma();
        HAL_NVIC_SetPriority(SAI1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(SAI1_IRQn);
    }
}
extern "C" void HAL_SAI_MspDeInit(SAI_HandleTypeDef* sai_handle)
{
    if(sai_handle->Instance == SAI1_Block_A)
    {
        __HAL_RCC_SAI1_CLK_DISABLE();
        sai_handles[0].DeinitPins();
    }
    else if(sai_handle->Instance == SAI2_Block_A)
    {
        __HAL_RCC_SAI2_CLK_DISABLE();
        sai_handles[1].DeinitPins();
    }
}


// ================================================================
// ISRs and event handlers
// ================================================================

void DMA1_Stream0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&sai_handles[0].sai_a_dma_handle_);
}
void DMA1_Stream1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&sai_handles[0].sai_b_dma_handle_);
}
void DMA1_Stream2_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&sai_handles[1].sai_a_dma_handle_);
}
void DMA1_Stream3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&sai_handles[1].sai_b_dma_handle_);
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef* hsai)
{
    // Get instance
    if(hsai->Instance == SAI1_Block_A || hsai->Instance == SAI1_Block_B)
    {
        sai_handles[0].TestCallback(0);
    }
    else if(hsai->Instance == SAI2_Block_A || hsai->Instance == SAI2_Block_B)
    {
        sai_handles[1].TestCallback(sai_handles[1].buff_size_/2);
    }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef* hsai) {}


// ================================================================
// SaiHandle -> SaiHandle::Pimpl
// ================================================================

SaiHandle::Result SaiHandle::Init(const Config& config)
{
    return pimpl_->Init(config);
}
const SaiHandle::Config& SaiHandle::GetConfig() const
{
    return pimpl_->GetConfig();
}

SaiHandle::Result
SaiHandle::StartDma(int32_t* buffer_rx, int32_t* buffer_tx, size_t size)
{
    return pimpl_->StartDmaTransfer(buffer_rx, buffer_tx, size);
}

SaiHandle::Result SaiHandle::StopDma()
{
    return pimpl_->StopDmaTransfer();
}


} // namespace daisy


// ================================================================
// OLD MESSY C SAI -- DELETE THIS AND BELOW
// ================================================================

// TODO:
// There is a lot of redundant duplication between sai1 and sai2,
//

//SAI_HandleTypeDef hsai_BlockA1;
//SAI_HandleTypeDef hsai_BlockB1;
//SAI_HandleTypeDef hsai_BlockA2;
//SAI_HandleTypeDef hsai_BlockB2;
//DMA_HandleTypeDef hdma_sai1_a;
//DMA_HandleTypeDef hdma_sai1_b;
//
//DMA_HandleTypeDef hdma_sai2_a;
//DMA_HandleTypeDef hdma_sai2_b;
//
//static dsy_sai_handle sai_handle;
//
////static void dsy_sai_blocks_init(SAI_HandleTypeDef* blockA, SAI_HandleTypeDef* blockB);
//static void dsy_sai_blocks_init(uint8_t            sai_idx,
//                                SAI_HandleTypeDef* blockA,
//                                SAI_HandleTypeDef* blockB);
//static void dsy_sai1_init();
//static void dsy_sai2_init();
//
//void dsy_sai_init(dsy_audio_sai        init,
//                  dsy_audio_samplerate sr[2],
//                  dsy_audio_bitdepth   bitdepth[2],
//                  dsy_audio_sync       sync_config[2],
//                  dsy_gpio_pin*        sai1_pin_list,
//                  dsy_gpio_pin*        sai2_pin_list)
//{
//    for(uint16_t i = 0; i < DSY_SAI_PIN_LAST; i++)
//    {
//        sai_handle.sai1_pin_config[i] = sai1_pin_list[i];
//        sai_handle.sai2_pin_config[i] = sai2_pin_list[i];
//    }
//    for(uint8_t i = 0; i < 2; i++)
//    {
//        sai_handle.samplerate[i]  = sr[i];
//        sai_handle.bitdepth[i]    = bitdepth[i];
//        sai_handle.sync_config[i] = sync_config[i];
//    }
//    sai_handle.init = init;
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
//}
//void dsy_sai_init_from_handle(dsy_sai_handle* hsai)
//{
//    sai_handle = *hsai;
//
//    // Set Instance for all Blocks
//    hsai_BlockA1.Instance = SAI1_Block_A;
//    hsai_BlockB1.Instance = SAI1_Block_B;
//    hsai_BlockA2.Instance = SAI2_Block_A;
//    hsai_BlockB2.Instance = SAI2_Block_B;
//    switch(sai_handle.init)
//    {
//        case DSY_AUDIO_INIT_SAI1:
//            dsy_sai_blocks_init(DSY_SAI_1, &hsai_BlockA1, &hsai_BlockB1);
//            break;
//        case DSY_AUDIO_INIT_SAI2:
//            //            dsy_sai_blocks_init(DSY_SAI_2, &hsai_BlockA2, &hsai_BlockB2);
//            dsy_sai2_init();
//            break;
//        case DSY_AUDIO_INIT_BOTH:
//            dsy_sai_blocks_init(DSY_SAI_1, &hsai_BlockA1, &hsai_BlockB1);
//            //            dsy_sai_blocks_init(DSY_SAI_2, &hsai_BlockA2, &hsai_BlockB2);
//            dsy_sai2_init();
//            break;
//        case DSY_AUDIO_INIT_NONE: break;
//        default: break;
//    }
//
//    //    switch(sai_handle.init)
//    //    {
//    //        case DSY_AUDIO_INIT_SAI1: dsy_sai1_init(); break;
//    //        case DSY_AUDIO_INIT_SAI2: dsy_sai2_init(); break;
//    //        case DSY_AUDIO_INIT_BOTH:
//    //            dsy_sai1_init();
//    //            dsy_sai2_init();
//    //            break;
//    //        case DSY_AUDIO_INIT_NONE: break;
//    //        default: break;
//    //    }
//}
//
//static void dsy_sai_blocks_init(uint8_t            sai_idx,
//                                SAI_HandleTypeDef* blockA,
//                                SAI_HandleTypeDef* blockB)
//{
//    // Block A
//    uint8_t  bd;
//    uint32_t protocol;
//    switch(sai_handle.bitdepth[sai_idx])
//    {
//        case DSY_AUDIO_BITDEPTH_16:
//            bd       = SAI_PROTOCOL_DATASIZE_16BIT;
//            protocol = SAI_I2S_STANDARD;
//            break;
//        case DSY_AUDIO_BITDEPTH_24:
//            bd       = SAI_PROTOCOL_DATASIZE_24BIT;
//            protocol = SAI_I2S_MSBJUSTIFIED;
//            break;
//        default:
//            bd       = SAI_PROTOCOL_DATASIZE_16BIT;
//            protocol = SAI_I2S_STANDARD;
//            break;
//    }
//    switch(sai_handle.sync_config[sai_idx])
//    {
//        case DSY_AUDIO_SYNC_SLAVE:
//            blockA->Init.AudioMode
//                = sai_handle.a_direction[sai_idx] == DSY_AUDIO_RX
//                      ? SAI_MODESLAVE_RX
//                      : SAI_MODESLAVE_TX;
//            break;
//        case DSY_AUDIO_SYNC_MASTER:
//            blockA->Init.AudioMode
//                = sai_handle.a_direction[sai_idx] == DSY_AUDIO_RX
//                      ? SAI_MODEMASTER_RX
//                      : SAI_MODEMASTER_TX;
//            break;
//        default: hsai_BlockA1.Init.AudioMode = SAI_MODEMASTER_RX; break;
//    }
//
//    blockA->Init.Synchro = SAI_ASYNCHRONOUS;
//
//    blockA->Init.OutputDrive    = SAI_OUTPUTDRIVE_DISABLE;
//    blockA->Init.NoDivider      = SAI_MASTERDIVIDER_ENABLE;
//    blockA->Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_EMPTY;
//    blockA->Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;
//    blockA->Init.SynchroExt     = SAI_SYNCEXT_DISABLE;
//    blockA->Init.MonoStereoMode = SAI_STEREOMODE;
//    blockA->Init.CompandingMode = SAI_NOCOMPANDING;
//    if(HAL_SAI_InitProtocol(blockA, protocol, bd, 2) != HAL_OK)
//    {
//        //Error_Handler();
//    }
//
//    blockB->Init.AudioMode = sai_handle.b_direction[sai_idx] == DSY_AUDIO_RX
//                                 ? SAI_MODESLAVE_RX
//                                 : SAI_MODESLAVE_TX;
//    //	hsai_BlockB1.Init.AudioMode		 = SAI_MODESLAVE_TX;
//    //    hsai_BlockB1.Init.AudioMode      = SAI_MODESLAVE_RX;
//    blockB->Init.Synchro        = SAI_SYNCHRONOUS;
//    blockB->Init.OutputDrive    = SAI_OUTPUTDRIVE_DISABLE;
//    blockB->Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_EMPTY;
//    blockB->Init.SynchroExt     = SAI_SYNCEXT_DISABLE;
//    blockB->Init.MonoStereoMode = SAI_STEREOMODE;
//    blockB->Init.CompandingMode = SAI_NOCOMPANDING;
//    blockB->Init.TriState       = SAI_OUTPUT_NOTRELEASED;
//
//    if(HAL_SAI_InitProtocol(blockB, protocol, bd, 2) != HAL_OK)
//    {
//        //Error_Handler();
//    }
//}
//
//
///* SAI1 init function */
//static void dsy_sai1_init()
//{
//    uint8_t  bd;
//    uint32_t protocol;
//    switch(sai_handle.bitdepth[DSY_SAI_1])
//    {
//        case DSY_AUDIO_BITDEPTH_16:
//            bd       = SAI_PROTOCOL_DATASIZE_16BIT;
//            protocol = SAI_I2S_STANDARD;
//            break;
//        case DSY_AUDIO_BITDEPTH_24:
//            bd       = SAI_PROTOCOL_DATASIZE_24BIT;
//            protocol = SAI_I2S_MSBJUSTIFIED;
//            break;
//        default:
//            bd       = SAI_PROTOCOL_DATASIZE_16BIT;
//            protocol = SAI_I2S_STANDARD;
//            break;
//    }
//    switch(sai_handle.sync_config[DSY_SAI_1])
//    {
//        case DSY_AUDIO_SYNC_SLAVE:
//            hsai_BlockA1.Init.AudioMode
//                = sai_handle.a_direction[DSY_SAI_1] == DSY_AUDIO_RX
//                      ? SAI_MODESLAVE_RX
//                      : SAI_MODESLAVE_TX;
//            break;
//        case DSY_AUDIO_SYNC_MASTER:
//            hsai_BlockA1.Init.AudioMode
//                = sai_handle.a_direction[DSY_SAI_1] == DSY_AUDIO_RX
//                      ? SAI_MODEMASTER_RX
//                      : SAI_MODEMASTER_TX;
//            break;
//        default: hsai_BlockA1.Init.AudioMode = SAI_MODEMASTER_RX; break;
//    }
//    hsai_BlockA1.Instance            = SAI1_Block_A;
//    hsai_BlockA1.Init.Synchro        = SAI_ASYNCHRONOUS;
//    hsai_BlockA1.Init.OutputDrive    = SAI_OUTPUTDRIVE_DISABLE;
//    hsai_BlockA1.Init.NoDivider      = SAI_MASTERDIVIDER_ENABLE;
//    hsai_BlockA1.Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_EMPTY;
//    hsai_BlockA1.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;
//    hsai_BlockA1.Init.SynchroExt     = SAI_SYNCEXT_DISABLE;
//    hsai_BlockA1.Init.MonoStereoMode = SAI_STEREOMODE;
//    hsai_BlockA1.Init.CompandingMode = SAI_NOCOMPANDING;
//    if(HAL_SAI_InitProtocol(&hsai_BlockA1, protocol, bd, 2) != HAL_OK)
//    {
//        //Error_Handler();
//    }
//
//    hsai_BlockB1.Instance = SAI1_Block_B;
//    hsai_BlockB1.Init.AudioMode
//        = sai_handle.b_direction[DSY_SAI_1] == DSY_AUDIO_RX ? SAI_MODESLAVE_RX
//                                                            : SAI_MODESLAVE_TX;
//    //	hsai_BlockB1.Init.AudioMode		 = SAI_MODESLAVE_TX;
//    //    hsai_BlockB1.Init.AudioMode      = SAI_MODESLAVE_RX;
//    hsai_BlockB1.Init.Synchro        = SAI_SYNCHRONOUS;
//    hsai_BlockB1.Init.OutputDrive    = SAI_OUTPUTDRIVE_DISABLE;
//    hsai_BlockB1.Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_EMPTY;
//    hsai_BlockB1.Init.SynchroExt     = SAI_SYNCEXT_DISABLE;
//    hsai_BlockB1.Init.MonoStereoMode = SAI_STEREOMODE;
//    hsai_BlockB1.Init.CompandingMode = SAI_NOCOMPANDING;
//    hsai_BlockB1.Init.TriState       = SAI_OUTPUT_NOTRELEASED;
//
//    if(HAL_SAI_InitProtocol(&hsai_BlockB1, protocol, bd, 2) != HAL_OK)
//    {
//        //Error_Handler();
//    }
//}
///* SAI2 init function */
//static void dsy_sai2_init()
//{
//    uint8_t  bd;
//    uint32_t protocol;
//    switch(sai_handle.bitdepth[DSY_SAI_2])
//    {
//        case DSY_AUDIO_BITDEPTH_16:
//            bd       = SAI_PROTOCOL_DATASIZE_16BIT;
//            protocol = SAI_I2S_STANDARD;
//            break;
//        case DSY_AUDIO_BITDEPTH_24:
//            bd       = SAI_PROTOCOL_DATASIZE_24BIT;
//            protocol = SAI_I2S_MSBJUSTIFIED;
//            break;
//        default:
//            bd       = SAI_PROTOCOL_DATASIZE_16BIT;
//            protocol = SAI_I2S_STANDARD;
//            break;
//    }
//    hsai_BlockA2.Init.AudioMode      = SAI_MODESLAVE_TX;
//    hsai_BlockA2.Instance            = SAI2_Block_A;
//    hsai_BlockA2.Init.Synchro        = SAI_SYNCHRONOUS;
//    hsai_BlockA2.Init.OutputDrive    = SAI_OUTPUTDRIVE_DISABLE;
//    hsai_BlockA2.Init.NoDivider      = SAI_MASTERDIVIDER_ENABLE;
//    hsai_BlockA2.Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_EMPTY;
//    hsai_BlockA2.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_48K;
//    hsai_BlockA2.Init.SynchroExt     = SAI_SYNCEXT_DISABLE;
//    hsai_BlockA2.Init.MonoStereoMode = SAI_STEREOMODE;
//    hsai_BlockA2.Init.CompandingMode = SAI_NOCOMPANDING;
//    if(HAL_SAI_InitProtocol(&hsai_BlockA2, protocol, bd, 2) != HAL_OK)
//    {
//        //Error_Handler();
//    }
//
//    hsai_BlockB2.Instance            = SAI2_Block_B;
//    hsai_BlockB2.Init.AudioMode      = SAI_MODEMASTER_RX;
//    hsai_BlockB2.Init.Synchro        = SAI_ASYNCHRONOUS;
//    hsai_BlockB2.Init.OutputDrive    = SAI_OUTPUTDRIVE_DISABLE;
//    hsai_BlockB2.Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_EMPTY;
//    hsai_BlockB2.Init.SynchroExt     = SAI_SYNCEXT_DISABLE;
//    hsai_BlockB2.Init.MonoStereoMode = SAI_STEREOMODE;
//    hsai_BlockB2.Init.CompandingMode = SAI_NOCOMPANDING;
//    hsai_BlockB2.Init.TriState       = SAI_OUTPUT_NOTRELEASED;
//
//    if(HAL_SAI_InitProtocol(&hsai_BlockB2, protocol, bd, 2) != HAL_OK)
//    {
//        //Error_Handler();
//    }
//}
//static uint32_t SAI1_client = 0;
//static uint32_t SAI2_client = 0;
//
//// Alternate Function Notes:
//// - All SAI1 for all boards so far use AF6
//// - All SAI2 for all boards except for PA2 on SeedRev2+ use AF10, PA2 uses AF8
//void HAL_SAI_MspInit(SAI_HandleTypeDef* hsai)
//{
//    /* GPIO Ports Clock Enable */
//    __HAL_RCC_GPIOE_CLK_ENABLE();
//    __HAL_RCC_GPIOB_CLK_ENABLE();
//    __HAL_RCC_GPIOG_CLK_ENABLE();
//    __HAL_RCC_GPIOD_CLK_ENABLE();
//    __HAL_RCC_GPIOC_CLK_ENABLE();
//    __HAL_RCC_GPIOA_CLK_ENABLE();
//    __HAL_RCC_GPIOI_CLK_ENABLE();
//    __HAL_RCC_GPIOH_CLK_ENABLE();
//    __HAL_RCC_GPIOF_CLK_ENABLE();
//
//    GPIO_InitTypeDef GPIO_InitStruct;
//    /* SAI1 */
//    if(hsai->Instance == SAI1_Block_A)
//    {
//        /* SAI1 clock enable */
//        if(SAI1_client == 0)
//        {
//            __HAL_RCC_SAI1_CLK_ENABLE();
//        }
//        SAI1_client++;
//        for(uint8_t i = 0; i < DSY_SAI_PIN_LAST; i++)
//        {
//            GPIO_TypeDef* port;
//            if(i != DSY_SAI_PIN_MCLK
//               || sai_handle.sync_config[DSY_SAI_1] == DSY_AUDIO_SYNC_MASTER)
//            {
//                dsy_gpio_pin* p;
//                p                     = &sai_handle.sai1_pin_config[i];
//                port                  = dsy_hal_map_get_port(p);
//                GPIO_InitStruct.Pin   = dsy_hal_map_get_pin(p);
//                GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
//                GPIO_InitStruct.Pull  = GPIO_NOPULL;
//                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//                GPIO_InitStruct.Alternate
//                    = GPIO_AF6_SAI1; // All SAI1 Pins so far use this AF.
//                HAL_GPIO_Init(port, &GPIO_InitStruct);
//            }
//        }
//
//        if(hsai->Init.AudioMode == SAI_MODESLAVE_RX
//           || hsai->Init.AudioMode == SAI_MODEMASTER_RX)
//        {
//            hdma_sai1_a.Init.Direction = DMA_PERIPH_TO_MEMORY;
//        }
//        else
//        {
//            hdma_sai1_a.Init.Direction = DMA_MEMORY_TO_PERIPH;
//        }
//        hdma_sai1_a.Instance                 = DMA1_Stream0;
//        hdma_sai1_a.Init.Request             = DMA_REQUEST_SAI1_A;
//        hdma_sai1_a.Init.PeriphInc           = DMA_PINC_DISABLE;
//        hdma_sai1_a.Init.MemInc              = DMA_MINC_ENABLE;
//        hdma_sai1_a.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
//        hdma_sai1_a.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
//        hdma_sai1_a.Init.Mode                = DMA_CIRCULAR;
//        hdma_sai1_a.Init.Priority            = DMA_PRIORITY_HIGH;
//        hdma_sai1_a.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
//        if(HAL_DMA_Init(&hdma_sai1_a) != HAL_OK)
//        {
//            //Error_Handler();
//        }
//
//        /* Several peripheral DMA handle pointers point to the same DMA handle.
//     Be aware that there is only one channel to perform all the requested DMAs. */
//        __HAL_LINKDMA(hsai, hdmarx, hdma_sai1_a);
//        __HAL_LINKDMA(hsai, hdmatx, hdma_sai1_a);
//    }
//    if(hsai->Instance == SAI1_Block_B)
//    {
//        /* SAI1 clock enable */
//        if(SAI1_client == 0)
//        {
//            __HAL_RCC_SAI1_CLK_ENABLE();
//        }
//        SAI1_client++;
//
//        /* Peripheral DMA init*/
//
//        if(hsai->Init.AudioMode == SAI_MODESLAVE_RX
//           || hsai->Init.AudioMode == SAI_MODEMASTER_RX)
//        {
//            hdma_sai1_b.Init.Direction = DMA_PERIPH_TO_MEMORY;
//        }
//        else
//        {
//            hdma_sai1_b.Init.Direction = DMA_MEMORY_TO_PERIPH;
//        }
//
//        hdma_sai1_b.Instance                 = DMA1_Stream1;
//        hdma_sai1_b.Init.Request             = DMA_REQUEST_SAI1_B;
//        hdma_sai1_b.Init.PeriphInc           = DMA_PINC_DISABLE;
//        hdma_sai1_b.Init.MemInc              = DMA_MINC_ENABLE;
//        hdma_sai1_b.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
//        hdma_sai1_b.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
//        hdma_sai1_b.Init.Mode                = DMA_CIRCULAR;
//        hdma_sai1_b.Init.Priority            = DMA_PRIORITY_HIGH;
//        hdma_sai1_b.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
//        if(HAL_DMA_Init(&hdma_sai1_b) != HAL_OK)
//        {
//            //Error_Handler();
//        }
//
//        /* Several peripheral DMA handle pointers point to the same DMA handle.
//     Be aware that there is only one channel to perform all the requested DMAs. */
//        __HAL_LINKDMA(hsai, hdmarx, hdma_sai1_b);
//        __HAL_LINKDMA(hsai, hdmatx, hdma_sai1_b);
//    }
//    /* SAI2 */
//    if(hsai->Instance == SAI2_Block_A)
//    {
//        /* SAI2 clock enable */
//        if(SAI2_client == 0)
//        {
//            __HAL_RCC_SAI2_CLK_ENABLE();
//        }
//        SAI2_client++;
//
//        if(hsai->Init.AudioMode == SAI_MODESLAVE_RX
//           || hsai->Init.AudioMode == SAI_MODEMASTER_RX)
//        {
//            hdma_sai2_a.Init.Direction = DMA_PERIPH_TO_MEMORY;
//        }
//        else
//        {
//            hdma_sai2_a.Init.Direction = DMA_MEMORY_TO_PERIPH;
//        }
//        hdma_sai2_a.Instance                 = DMA1_Stream3;
//        hdma_sai2_a.Init.Request             = DMA_REQUEST_SAI2_A;
//        hdma_sai2_a.Init.PeriphInc           = DMA_PINC_DISABLE;
//        hdma_sai2_a.Init.MemInc              = DMA_MINC_ENABLE;
//        hdma_sai2_a.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
//        hdma_sai2_a.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
//        hdma_sai2_a.Init.Mode                = DMA_CIRCULAR;
//        hdma_sai2_a.Init.Priority            = DMA_PRIORITY_HIGH;
//        hdma_sai2_a.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
//        if(HAL_DMA_Init(&hdma_sai2_a) != HAL_OK)
//        {
//            //Error_Handler();
//        }
//        //        pSyncConfig.SyncSignalID  = HAL_DMAMUX1_SYNC_DMAMUX1_CH1_EVT;
//        //        pSyncConfig.SyncPolarity  = HAL_DMAMUX_SYNC_RISING;
//        //        pSyncConfig.SyncEnable    = ENABLE;
//        //        pSyncConfig.EventEnable   = DISABLE;
//        //        pSyncConfig.RequestNumber = 1;
//        //        if(HAL_DMAEx_ConfigMuxSync(&hdma_sai2_a, &pSyncConfig) != HAL_OK)
//        //        {
//        //            //Error_Handler();
//        //        }
//
//        /* Several peripheral DMA handle pointers point to the same DMA handle.
//     Be aware that there is only one channel to perform all the requested DMAs. */
//        __HAL_LINKDMA(hsai, hdmarx, hdma_sai2_a);
//        __HAL_LINKDMA(hsai, hdmatx, hdma_sai2_a);
//    }
//    if(hsai->Instance == SAI2_Block_B)
//    {
//        /* SAI2 clock enable */
//        if(SAI2_client == 0)
//        {
//            __HAL_RCC_SAI2_CLK_ENABLE();
//        }
//        SAI2_client++;
//
//        // New style config:
//        for(uint8_t i = 0; i < DSY_SAI_PIN_LAST; i++)
//        {
//            GPIO_TypeDef* port;
//            if(i != DSY_SAI_PIN_MCLK
//               || sai_handle.sync_config[DSY_SAI_2] == DSY_AUDIO_SYNC_MASTER)
//            {
//                dsy_gpio_pin* p;
//                p                     = &sai_handle.sai2_pin_config[i];
//                port                  = dsy_hal_map_get_port(p);
//                GPIO_InitStruct.Pin   = dsy_hal_map_get_pin(p);
//                GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
//                GPIO_InitStruct.Pull  = GPIO_NOPULL;
//                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//                GPIO_InitStruct.Alternate
//                    = i == DSY_SAI_PIN_SCK ? GPIO_AF8_SAI2 : GPIO_AF10_SAI2;
//                HAL_GPIO_Init(port, &GPIO_InitStruct);
//            }
//        }
//
//        if(hsai->Init.AudioMode == SAI_MODESLAVE_RX
//           || hsai->Init.AudioMode == SAI_MODEMASTER_RX)
//        {
//            hdma_sai2_b.Init.Direction = DMA_PERIPH_TO_MEMORY;
//        }
//        else
//        {
//            hdma_sai2_b.Init.Direction = DMA_MEMORY_TO_PERIPH;
//        }
//
//        hdma_sai2_b.Instance                 = DMA1_Stream4;
//        hdma_sai2_b.Init.Request             = DMA_REQUEST_SAI2_B;
//        hdma_sai2_b.Init.PeriphInc           = DMA_PINC_DISABLE;
//        hdma_sai2_b.Init.MemInc              = DMA_MINC_ENABLE;
//        hdma_sai2_b.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
//        hdma_sai2_b.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
//        hdma_sai2_b.Init.Mode                = DMA_CIRCULAR;
//        hdma_sai2_b.Init.Priority            = DMA_PRIORITY_HIGH;
//        hdma_sai2_b.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
//        if(HAL_DMA_Init(&hdma_sai2_b) != HAL_OK)
//        {
//            //Error_Handler();
//        }
//
//        //        pSyncConfig.SyncSignalID  = HAL_DMAMUX1_SYNC_DMAMUX1_CH1_EVT;
//        //        pSyncConfig.SyncPolarity  = HAL_DMAMUX_SYNC_RISING;
//        //        pSyncConfig.SyncEnable    = ENABLE;
//        //        pSyncConfig.EventEnable   = DISABLE;
//        //        pSyncConfig.RequestNumber = 1;
//        //        if(HAL_DMAEx_ConfigMuxSync(&hdma_sai2_b, &pSyncConfig) != HAL_OK)
//        //        {
//        //            //Error_Handler();
//        //        }
//        /* Several peripheral DMA handle pointers point to the same DMA handle.
//     Be aware that there is only one channel to perform all the requested DMAs. */
//        __HAL_LINKDMA(hsai, hdmarx, hdma_sai2_b);
//        __HAL_LINKDMA(hsai, hdmatx, hdma_sai2_b);
//    }
//}
//
//void HAL_SAI_MspDeInit(SAI_HandleTypeDef* hsai)
//{
//    /* SAI1 */
//    if(hsai->Instance == SAI1_Block_A)
//    {
//        SAI1_client--;
//        if(SAI1_client == 0)
//        {
//            __HAL_RCC_SAI1_CLK_DISABLE();
//        }
//        for(uint8_t i = 0; i < DSY_SAI_PIN_LAST; i++)
//        {
//            GPIO_TypeDef* port;
//            uint16_t      pin;
//            if(i != DSY_SAI_PIN_MCLK
//               || sai_handle.sync_config[DSY_SAI_1] == DSY_AUDIO_SYNC_MASTER)
//            {
//                dsy_gpio_pin* p;
//                p    = &sai_handle.sai1_pin_config[i];
//                port = dsy_hal_map_get_port(p);
//                pin  = dsy_hal_map_get_pin(p);
//                HAL_GPIO_DeInit(port, pin);
//            }
//        }
//        HAL_DMA_DeInit(hsai->hdmarx);
//        HAL_DMA_DeInit(hsai->hdmatx);
//    }
//    if(hsai->Instance == SAI1_Block_B)
//    {
//        SAI1_client--;
//        if(SAI1_client == 0)
//        {
//            __HAL_RCC_SAI1_CLK_DISABLE();
//        }
//        for(uint8_t i = 0; i < DSY_SAI_PIN_LAST; i++)
//        {
//            GPIO_TypeDef* port;
//            uint16_t      pin;
//            if(i != DSY_SAI_PIN_MCLK
//               || sai_handle.sync_config[DSY_SAI_1] == DSY_AUDIO_SYNC_MASTER)
//            {
//                dsy_gpio_pin* p;
//                p    = &sai_handle.sai1_pin_config[i];
//                port = dsy_hal_map_get_port(p);
//                pin  = dsy_hal_map_get_pin(p);
//                HAL_GPIO_DeInit(port, pin);
//            }
//        }
//
//        HAL_DMA_DeInit(hsai->hdmarx);
//        HAL_DMA_DeInit(hsai->hdmatx);
//    }
//    /* SAI2 */
//    if(hsai->Instance == SAI2_Block_A)
//    {
//        SAI2_client--;
//        if(SAI2_client == 0)
//        {
//            __HAL_RCC_SAI2_CLK_DISABLE();
//        }
//        for(uint8_t i = 0; i < DSY_SAI_PIN_LAST; i++)
//        {
//            GPIO_TypeDef* port;
//            uint16_t      pin;
//            if(i != DSY_SAI_PIN_MCLK
//               || sai_handle.sync_config[DSY_SAI_2] == DSY_AUDIO_SYNC_MASTER)
//            {
//                dsy_gpio_pin* p;
//                p    = &sai_handle.sai2_pin_config[i];
//                port = dsy_hal_map_get_port(p);
//                pin  = dsy_hal_map_get_pin(p);
//                //				port = (GPIO_TypeDef*)
//                //					gpio_hal_port_map[sai_handle.sai2_pin_config[i].port];
//                //				pin
//                //					= gpio_hal_pin_map[sai_handle.sai2_pin_config[i].pin];
//                HAL_GPIO_DeInit(port, pin);
//            }
//        }
//    }
//    if(hsai->Instance == SAI2_Block_B)
//    {
//        SAI2_client--;
//        if(SAI2_client == 0)
//        {
//            __HAL_RCC_SAI2_CLK_DISABLE();
//        }
//        for(uint8_t i = 0; i < DSY_SAI_PIN_LAST; i++)
//        {
//            GPIO_TypeDef* port;
//            uint16_t      pin;
//            if(i != DSY_SAI_PIN_MCLK
//               || sai_handle.sync_config[DSY_SAI_2] == DSY_AUDIO_SYNC_MASTER)
//            {
//                dsy_gpio_pin* p;
//                p    = &sai_handle.sai2_pin_config[i];
//                port = dsy_hal_map_get_port(p);
//                pin  = dsy_hal_map_get_pin(p);
//                //				port = (GPIO_TypeDef*)
//                //					gpio_hal_port_map[sai_handle.sai2_pin_config[i].port];
//                //				pin
//                //					= gpio_hal_pin_map[sai_handle.sai2_pin_config[i].pin];
//                HAL_GPIO_DeInit(port, pin);
//            }
//        }
//    }
//}
