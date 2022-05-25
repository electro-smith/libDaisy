#include "spiMultislave.h"

namespace daisy
{
SpiHandle::Result MultiSlaveSpiHandle::Init(const Config& config)
{
    if(config.num_devices >= max_num_devices_)
        return SpiHandle::Result::ERR;

    config_ = config;

    for(size_t i = 0; i < config_.num_devices; i++)
    {
        nss_pins[i].pin  = config_.pin_config.nss[i];
        nss_pins[i].mode = DSY_GPIO_MODE_OUTPUT_PP;
        nss_pins[i].pull = DSY_GPIO_NOPULL;
        dsy_gpio_init(&nss_pins[i]);
        DisableDevice(i);
    }

    current_dma_transfer_.Invalidate();

    SpiHandle::Config spi_config;
    spi_config.baud_prescaler  = config.baud_prescaler;
    spi_config.clock_phase     = config.clock_phase;
    spi_config.clock_polarity  = config.clock_polarity;
    spi_config.datasize        = config.datasize;
    spi_config.direction       = config.direction;
    spi_config.mode            = SpiHandle::Config::Mode::MASTER;
    spi_config.nss             = SpiHandle::Config::NSS::SOFT;
    spi_config.periph          = config.periph;
    spi_config.pin_config.miso = config.pin_config.miso;
    spi_config.pin_config.mosi = config.pin_config.mosi;
    spi_config.pin_config.sclk = config.pin_config.sclk;
    spi_config.pin_config.nss = {DSY_GPIOX, 0}; // we'll drive this by ourselves
    return spiHandle_.Init(spi_config);
}

SpiHandle::Result MultiSlaveSpiHandle::BlockingTransmit(size_t   device_index,
                                                        uint8_t* buff,
                                                        size_t   size,
                                                        uint32_t timeout)
{
    if(device_index >= config_.num_devices)
        return SpiHandle::Result::ERR;

    // wait for previous DMA transfer to complete
    while(current_dma_transfer_.IsValid()) {}

    EnableDevice(device_index);
    const auto result = spiHandle_.BlockingTransmit(buff, size, timeout);
    DisableDevice(device_index);
    return result;
}

SpiHandle::Result MultiSlaveSpiHandle::BlockingReceive(size_t   device_index,
                                                       uint8_t* buff,
                                                       uint16_t size,
                                                       uint32_t timeout)
{
    if(device_index >= config_.num_devices)
        return SpiHandle::Result::ERR;

    // wait for previous DMA transfer to complete
    while(current_dma_transfer_.IsValid()) {}

    EnableDevice(device_index);
    const auto result = spiHandle_.BlockingReceive(buff, size, timeout);
    DisableDevice(device_index);
    return result;
}

SpiHandle::Result
MultiSlaveSpiHandle::BlockingTransmitAndReceive(size_t   device_index,
                                                uint8_t* tx_buff,
                                                uint8_t* rx_buff,
                                                size_t   size,
                                                uint32_t timeout)
{
    if(device_index >= config_.num_devices)
        return SpiHandle::Result::ERR;

    // wait for previous DMA transfer to complete
    while(current_dma_transfer_.IsValid()) {}

    EnableDevice(device_index);
    const auto result = spiHandle_.BlockingTransmitAndReceive(
        tx_buff, rx_buff, size, timeout);
    DisableDevice(device_index);
    return result;
}

SpiHandle::Result MultiSlaveSpiHandle::DmaTransmit(
    size_t                              device_index,
    uint8_t*                            buff,
    size_t                              size,
    SpiHandle::StartCallbackFunctionPtr start_callback,
    SpiHandle::EndCallbackFunctionPtr   end_callback,
    void*                               callback_context)
{
    if(device_index >= config_.num_devices)
        return SpiHandle::Result::ERR;

    // wait for previous DMA transfer to complete
    while(current_dma_transfer_.IsValid()) {}

    current_dma_transfer_.device_index     = device_index;
    current_dma_transfer_.start_callback   = start_callback;
    current_dma_transfer_.end_callback     = end_callback;
    current_dma_transfer_.callback_context = callback_context;
    return spiHandle_.DmaTransmit(
        buff, size, &DmaStartCallback, &DmaEndCallback, this);
}

SpiHandle::Result MultiSlaveSpiHandle::DmaReceive(
    size_t                              device_index,
    uint8_t*                            buff,
    size_t                              size,
    SpiHandle::StartCallbackFunctionPtr start_callback,
    SpiHandle::EndCallbackFunctionPtr   end_callback,
    void*                               callback_context)
{
    if(device_index >= config_.num_devices)
        return SpiHandle::Result::ERR;

    // wait for previous DMA transfer to complete
    while(current_dma_transfer_.IsValid()) {}

    current_dma_transfer_.device_index     = device_index;
    current_dma_transfer_.start_callback   = start_callback;
    current_dma_transfer_.end_callback     = end_callback;
    current_dma_transfer_.callback_context = callback_context;
    return spiHandle_.DmaReceive(
        buff, size, &DmaStartCallback, &DmaEndCallback, this);
}

SpiHandle::Result MultiSlaveSpiHandle::DmaTransmitAndReceive(
    size_t                              device_index,
    uint8_t*                            tx_buff,
    uint8_t*                            rx_buff,
    size_t                              size,
    SpiHandle::StartCallbackFunctionPtr start_callback,
    SpiHandle::EndCallbackFunctionPtr   end_callback,
    void*                               callback_context)
{
    if(device_index >= config_.num_devices)
        return SpiHandle::Result::ERR;

    // wait for previous DMA transfer to complete
    while(current_dma_transfer_.IsValid()) {}

    current_dma_transfer_.device_index     = device_index;
    current_dma_transfer_.start_callback   = start_callback;
    current_dma_transfer_.end_callback     = end_callback;
    current_dma_transfer_.callback_context = callback_context;
    return spiHandle_.DmaTransmitAndReceive(
        tx_buff, rx_buff, size, &DmaStartCallback, &DmaEndCallback, this);
}

int MultiSlaveSpiHandle::CheckError()
{
    return spiHandle_.CheckError();
}

void MultiSlaveSpiHandle::EnableDevice(size_t device_index)
{
    dsy_gpio_write(&nss_pins[device_index], 0);
}

void MultiSlaveSpiHandle::DisableDevice(size_t device_index)
{
    dsy_gpio_write(&nss_pins[device_index], 1);
}

void MultiSlaveSpiHandle::DmaStartCallback(void* context)
{
    auto&      handle       = *reinterpret_cast<MultiSlaveSpiHandle*>(context);
    const auto device_index = handle.current_dma_transfer_.device_index;
    if(device_index >= 0)
    {
        handle.EnableDevice(device_index);
        if(handle.current_dma_transfer_.start_callback)
            handle.current_dma_transfer_.start_callback(
                handle.current_dma_transfer_.callback_context);
    }
}
void MultiSlaveSpiHandle::DmaEndCallback(void*             context,
                                         SpiHandle::Result result)
{
    auto&      handle       = *reinterpret_cast<MultiSlaveSpiHandle*>(context);
    const auto device_index = handle.current_dma_transfer_.device_index;
    handle.current_dma_transfer_.Invalidate();
    if(device_index >= 0)
    {
        handle.DisableDevice(device_index);
        if(handle.current_dma_transfer_.end_callback)
            handle.current_dma_transfer_.end_callback(
                handle.current_dma_transfer_.callback_context, result);
    }
}

} // namespace daisy