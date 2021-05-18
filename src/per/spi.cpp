#include "per/spi.h"
#include "util/hal_map.h"

// TODO
// - fix up rest of lib so that we can add a spi_handle map to the hal map
// - Add configuration for standard spi stuff.


using namespace daisy;

static void Error_Handler()
{
    asm("bkpt 255");
}

class SpiHandle::Impl
{
  public:
    Result Init(const Config& config);

    Result BlockingTransmit(uint8_t* buff, size_t size);

    SpiHandle::Config config_;
    SPI_HandleTypeDef hspi_;
};

// ================================================================
// Global references for the availabel UartHandler::Impl(s)
// ================================================================

static SpiHandle::Impl spi_handles[6];

SpiHandle::Impl* MapInstanceToHandle(SPI_TypeDef* instance)
{
    // map HAL instances
    constexpr SPI_TypeDef* instances[6] = {SPI1, SPI2, SPI3, SPI4, SPI5, SPI6};

    for(int i = 0; i < 6; i++)
    {
        if(instance == instances[i])
        {
            return &spi_handles[i];
        }
    }

    /* error */
    return NULL;
}

SpiHandle::Result SpiHandle::Impl::Init(const Config& config)
{
    config_ = config;

    SPI_TypeDef* periph;
    switch(config_.periph)
    {
        case Config::Peripheral::SPI_1: periph = SPI1; break;
        case Config::Peripheral::SPI_2: periph = SPI2; break;
        case Config::Peripheral::SPI_3: periph = SPI3; break;
        case Config::Peripheral::SPI_4: periph = SPI4; break;
        case Config::Peripheral::SPI_5: periph = SPI5; break;
        case Config::Peripheral::SPI_6: periph = SPI6; break;
        default: return Result::ERR;
    }

    uint32_t mode;
    switch(config_.mode)
    {
        case Config::Mode::MASTER: mode = SPI_MODE_MASTER; break;
        case Config::Mode::SLAVE: mode = SPI_MODE_SLAVE; break;
        default: return Result::ERR;
    }

    uint32_t direction;
    switch(config_.direction)
    {
        case Config::Direction::TWO_LINES:
            direction = SPI_DIRECTION_2LINES;
            break;
        case Config::Direction::TWO_LINES_TX_ONLY:
            direction = SPI_DIRECTION_2LINES_TXONLY;
            break;
        case Config::Direction::TWO_LINES_RX_ONLY:
            direction = SPI_DIRECTION_2LINES_RXONLY;
            break;
        case Config::Direction::ONE_LINE:
            direction = SPI_DIRECTION_1LINE;
            break;
        default: return Result::ERR;
    }

    // for some reason a datasize of 30 is encoded as 29
    // ie SPI_DATASIZE_5BIT == 4, etc.
    // we might also consider going the enum route for this one, but it'll be LONG
    uint32_t datasize = config_.datasize - 1;
    if(datasize < 3 || datasize > 31)
    {
        return Result::ERR;
    }

    uint32_t clock_polarity;
    switch(config_.clock_polarity)
    {
        case Config::ClockPolarity::LOW:
            clock_polarity = SPI_POLARITY_LOW;
            break;
        case Config::ClockPolarity::HIGH:
            clock_polarity = SPI_POLARITY_HIGH;
            break;
        default: return Result::ERR;
    }

    uint32_t clock_phase;
    switch(config_.clock_phase)
    {
        case Config::ClockPhase::ONE_EDGE: clock_phase = SPI_PHASE_1EDGE; break;
        case Config::ClockPhase::TWO_EDGE: clock_phase = SPI_PHASE_2EDGE; break;
        default: return Result::ERR;
    }

    uint32_t nss;
    switch(config_.nss)
    {
        case Config::NSS::SOFT: clock_phase = SPI_NSS_SOFT; break;
        case Config::NSS::HARD_INPUT: clock_phase = SPI_NSS_HARD_INPUT; break;
        case Config::NSS::HARD_OUTPUT: clock_phase = SPI_NSS_HARD_OUTPUT; break;
        default: return Result::ERR;
    }

    uint32_t baud_prescaler;
    switch(config_.baud_prescaler)
    {
        case Config::BaudPrescaler::BAUDRATEPRESCALER_2:
            baud_prescaler = SPI_BAUDRATEPRESCALER_2;
            break;
        case Config::BaudPrescaler::BAUDRATEPRESCALER_4:
            baud_prescaler = SPI_BAUDRATEPRESCALER_4;
            break;
        case Config::BaudPrescaler::BAUDRATEPRESCALER_8:
            baud_prescaler = SPI_BAUDRATEPRESCALER_8;
            break;
        case Config::BaudPrescaler::BAUDRATEPRESCALER_16:
            baud_prescaler = SPI_BAUDRATEPRESCALER_16;
            break;
        case Config::BaudPrescaler::BAUDRATEPRESCALER_32:
            baud_prescaler = SPI_BAUDRATEPRESCALER_32;
            break;
        case Config::BaudPrescaler::BAUDRATEPRESCALER_64:
            baud_prescaler = SPI_BAUDRATEPRESCALER_64;
            break;
        case Config::BaudPrescaler::BAUDRATEPRESCALER_128:
            baud_prescaler = SPI_BAUDRATEPRESCALER_128;
            break;
        case Config::BaudPrescaler::BAUDRATEPRESCALER_256:
            baud_prescaler = SPI_BAUDRATEPRESCALER_256;
            break;
        default: return Result::ERR;
    }

    hspi_.Instance               = periph;
    hspi_.Init.Mode              = mode;
    hspi_.Init.Direction         = direction;
    hspi_.Init.DataSize          = datasize;
    hspi_.Init.CLKPolarity       = clock_polarity;
    hspi_.Init.CLKPhase          = clock_phase;
    hspi_.Init.NSS               = nss;
    hspi_.Init.BaudRatePrescaler = baud_prescaler;
    hspi_.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    hspi_.Init.TIMode            = SPI_TIMODE_DISABLE;
    hspi_.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    hspi_.Init.CRCPolynomial     = 0x0;
    hspi_.Init.NSSPMode          = SPI_NSS_PULSE_ENABLE;
    hspi_.Init.NSSPolarity       = SPI_NSS_POLARITY_LOW;
    hspi_.Init.FifoThreshold     = SPI_FIFO_THRESHOLD_01DATA;
    hspi_.Init.TxCRCInitializationPattern
        = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi_.Init.RxCRCInitializationPattern
        = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
    hspi_.Init.MasterSSIdleness        = SPI_MASTER_SS_IDLENESS_00CYCLE;
    hspi_.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
    hspi_.Init.MasterReceiverAutoSusp  = SPI_MASTER_RX_AUTOSUSP_DISABLE;
    hspi_.Init.MasterKeepIOState       = SPI_MASTER_KEEP_IO_STATE_DISABLE;
    hspi_.Init.IOSwap                  = SPI_IO_SWAP_DISABLE;
    if(HAL_SPI_Init(&hspi_) != HAL_OK)
    {
        Error_Handler();
        return SpiHandle::Result::ERR;
    }

    return SpiHandle::Result::OK;
}

SpiHandle::Result SpiHandle::Impl::BlockingTransmit(uint8_t* buff, size_t size)
{
    if(HAL_SPI_Transmit(&hspi_, buff, size, 100) != HAL_OK)
    {
        return SpiHandle::Result::ERR;
    }
    return SpiHandle::Result::OK;
}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{
    SpiHandle::Impl* handle = MapInstanceToHandle(spiHandle->Instance);
    // dsy_hal_map_gpio_clk_enable(handle->config_.pin_config.rx.port);
    // dsy_hal_map_gpio_clk_enable(handle->config_.pin_config.tx.port);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(spiHandle->Instance == SPI1)
    {
        /* USER CODE BEGIN SPI1_MspInit 0 */

        /* USER CODE END SPI1_MspInit 0 */
        /* SPI1 clock enable */
        __HAL_RCC_SPI1_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOG_CLK_ENABLE();
        /**SPI1 GPIO Configuration    
    PB5     ------> SPI1_MOSI
    PB4 (NJTRST)     ------> SPI1_MISO
    PG11     ------> SPI1_SCK
    PG10     ------> SPI1_NSS 
    */
        //        GPIO_InitStruct.Pin       = GPIO_PIN_5 | GPIO_PIN_4;
        switch(spiHandle->Init.Direction)
        {
            case SPI_DIRECTION_2LINES_TXONLY:
                GPIO_InitStruct.Pin = GPIO_PIN_5;
                break;
            case SPI_DIRECTION_2LINES_RXONLY:
                GPIO_InitStruct.Pin = GPIO_PIN_4;
                break;
            default: GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5; break;
        }
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        // Sck and CS
        GPIO_InitStruct.Pin = GPIO_PIN_11;
        if(spiHandle->Init.NSS != SPI_NSS_SOFT)
        {
            GPIO_InitStruct.Pin |= GPIO_PIN_10;
        }
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

        /* USER CODE BEGIN SPI1_MspInit 1 */

        /* USER CODE END SPI1_MspInit 1 */
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{
    SpiHandle::Impl* handle = MapInstanceToHandle(spiHandle->Instance);

    if(spiHandle->Instance == SPI1)
    {
        /* USER CODE BEGIN SPI1_MspDeInit 0 */

        /* USER CODE END SPI1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_SPI1_CLK_DISABLE();

        /**SPI1 GPIO Configuration    
    PB5     ------> SPI1_MOSI
    PB4 (NJTRST)     ------> SPI1_MISO
    PG11     ------> SPI1_SCK
    PG10     ------> SPI1_NSS 
    */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5 | GPIO_PIN_4);

        HAL_GPIO_DeInit(GPIOG, GPIO_PIN_11 | GPIO_PIN_10);

        /* USER CODE BEGIN SPI1_MspDeInit 1 */

        /* USER CODE END SPI1_MspDeInit 1 */
    }
}

// ======================================================================
// SpiHandler > SpiHandlePimpl
// ======================================================================

SpiHandle::Result SpiHandle::Init(const Config& config)
{
    pimpl_ = &spi_handles[int(config.periph)];
    return pimpl_->Init(config);
}

SpiHandle::Result SpiHandle::BlockingTransmit(uint8_t* buff, size_t size)
{
    return pimpl_->BlockingTransmit(buff, size);
}