#include "hid/usb.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "tusb.h"
#include "tusb_config.h"
#include "system.h"
#include "hid/logger.h"
#include "per/gpio.h"

using namespace daisy;

static void UsbErrorHandler();

bool usb_fs_hw_initialized = false;
bool usb_hs_hw_initialized = false;

// Prevents multiple calls to tud_task() from different contexts
static bool tud_task_running = false;

// TinyUSB specific globals
static UsbHandle::ReceiveCallback usb_rx_callback = nullptr;
static uint8_t                    rx_buffer[CFG_TUD_CDC_RX_BUFSIZE];

// Use TinyUSB by default
UsbHandle::UsbImpl UsbHandle::current_impl_ = UsbHandle::IMPL_TINYUSB;

// Simple USB specific globals
UsbHandle::ReceiveCallback rx_callback;

// Externs for IRQ Handlers
extern "C"
{
    // Globals from Cube generated version:
    USBD_HandleTypeDef       hUsbDeviceHS;
    USBD_HandleTypeDef       hUsbDeviceFS;
    extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
    extern PCD_HandleTypeDef hpcd_USB_OTG_HS;

    void DummyRxCallback(uint8_t *buf, uint32_t *size)
    {
        // Do Nothing
    }

    CDC_ReceiveCallback rxcallback;

    uint8_t usbd_mode = USBD_MODE_CDC;
}

// TinyUSB implementation functions
static void InitFS_TinyUSB()
{
    rx_callback = DummyRxCallback;
    if(!usb_fs_hw_initialized)
    {
        usb_fs_hw_initialized = true;
        if(USBD_Init(&hUsbDeviceFS, NULL, DEVICE_FS) != USBD_OK)
        {
            UsbErrorHandler();
        }
    }

    auto result = tud_init(BOARD_TUD_FS_RHPORT);
    if(!result)
    {
        UsbErrorHandler();
    }
}

static void InitHS_TinyUSB()
{
    rx_callback = DummyRxCallback;
    if(!usb_hs_hw_initialized)
    {
        usb_hs_hw_initialized = true;
        if(USBD_Init(&hUsbDeviceHS, NULL, DEVICE_HS) != USBD_OK)
        {
            UsbErrorHandler();
        }
    }
    auto result = tud_init(BOARD_TUD_HS_RHPORT);
    if(!result)
    {
        UsbErrorHandler();
    }
}

static void DeinitFS_TinyUSB()
{
    if(USBD_DeInit(&hUsbDeviceFS) != USBD_OK)
    {
        UsbErrorHandler();
    }
}

static void DeinitHS_TinyUSB()
{
    if(USBD_DeInit(&hUsbDeviceHS) != USBD_OK)
    {
        UsbErrorHandler();
    }
}

// Simple USB implementation functions
static void InitFS_Simple()
{
    rx_callback = DummyRxCallback;
    if(USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
    {
        UsbErrorHandler();
    }
    if(USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK)
    {
        UsbErrorHandler();
    }
    if(USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS)
       != USBD_OK)
    {
        UsbErrorHandler();
    }
    if(USBD_Start(&hUsbDeviceFS) != USBD_OK)
    {
        UsbErrorHandler();
    }
}

static void InitHS_Simple()
{
    // HS as FS
    if(USBD_Init(&hUsbDeviceHS, &HS_Desc, DEVICE_HS) != USBD_OK)
    {
        UsbErrorHandler();
    }
    if(USBD_RegisterClass(&hUsbDeviceHS, &USBD_CDC) != USBD_OK)
    {
        UsbErrorHandler();
    }
    if(USBD_CDC_RegisterInterface(&hUsbDeviceHS, &USBD_Interface_fops_HS)
       != USBD_OK)
    {
        UsbErrorHandler();
    }
    if(USBD_Start(&hUsbDeviceHS) != USBD_OK)
    {
        UsbErrorHandler();
    }
}

static void DeinitFS_Simple()
{
    if(USBD_DeInit(&hUsbDeviceFS) != USBD_OK)
    {
        UsbErrorHandler();
    }
}

static void DeinitHS_Simple()
{
    if(USBD_DeInit(&hUsbDeviceHS) != USBD_OK)
    {
        UsbErrorHandler();
    }
}

void UsbHandle::SetImplementation(UsbImpl impl)
{
    current_impl_ = impl;
}

void UsbHandle::RunTask()
{
    if(current_impl_ == IMPL_TINYUSB)
    {
        // Protect against reentrant calls
        if(tud_task_running)
            return;

        tud_task_running = true;
        tud_task();
        tud_task_running = false;
    }
    // Simple USB doesn't need a task function
}

void UsbHandle::Init(UsbPeriph dev)
{
    if(current_impl_ == IMPL_TINYUSB)
    {
        switch(dev)
        {
            case FS_INTERNAL: InitFS_TinyUSB(); break;
            case FS_EXTERNAL: InitHS_TinyUSB(); break;
            case FS_BOTH:
                InitHS_TinyUSB();
                InitFS_TinyUSB();
                break;
            default: break;
        }
    }
    else // IMPL_SIMPLE
    {
        switch(dev)
        {
            case FS_INTERNAL: InitFS_Simple(); break;
            case FS_EXTERNAL: InitHS_Simple(); break;
            case FS_BOTH:
                InitHS_Simple();
                InitFS_Simple();
                break;
            default: break;
        }
    }
    // Enable USB Regulator
    HAL_PWREx_EnableUSBVoltageDetector();
}

void UsbHandle::DeInit(UsbPeriph dev)
{
    if(current_impl_ == IMPL_TINYUSB)
    {
        switch(dev)
        {
            case FS_INTERNAL: DeinitFS_TinyUSB(); break;
            case FS_EXTERNAL: DeinitHS_TinyUSB(); break;
            case FS_BOTH:
                DeinitHS_TinyUSB();
                DeinitFS_TinyUSB();
                break;
            default: break;
        }
    }
    else // IMPL_SIMPLE
    {
        switch(dev)
        {
            case FS_INTERNAL: DeinitFS_Simple(); break;
            case FS_EXTERNAL: DeinitHS_Simple(); break;
            case FS_BOTH:
                DeinitHS_Simple();
                DeinitFS_Simple();
                break;
            default: break;
        }
    }
    // Disable USB Regulator
    HAL_PWREx_DisableUSBVoltageDetector();
}

static UsbHandle::Result Transmit_TinyUSB(uint8_t *buff, size_t size)
{
    // Check if buffer is valid and non-empty
    if(buff == nullptr || size == 0)
    {
        return UsbHandle::Result::ERR;
    }

    // Write the data
    size_t written = tud_cdc_write(buff, size);
    if(written != size)
    {
        return UsbHandle::Result::ERR;
    }

    // If we exactly filled a packet, send a ZLP to indicate completion
    if(size % CFG_TUD_CDC_EP_BUFSIZE == 0)
    {
        tud_cdc_write(NULL, 0);
    }

    // Flush the data
    tud_cdc_write_flush();
    return UsbHandle::Result::OK;
}

UsbHandle::Result UsbHandle::TransmitInternal(uint8_t *buff, size_t size)
{
    if(current_impl_ == IMPL_TINYUSB)
    {
        return Transmit_TinyUSB(buff, size);
    }
    else // IMPL_SIMPLE
    {
        return CDC_Transmit_FS(buff, size) == USBD_OK ? Result::OK
                                                      : Result::ERR;
    }
}

UsbHandle::Result UsbHandle::TransmitExternal(uint8_t *buff, size_t size)
{
    if(current_impl_ == IMPL_TINYUSB)
    {
        return Transmit_TinyUSB(buff, size);
    }
    else // IMPL_SIMPLE
    {
        return CDC_Transmit_HS(buff, size) == USBD_OK ? Result::OK
                                                      : Result::ERR;
    }
}

void UsbHandle::SetReceiveCallback(ReceiveCallback cb, UsbPeriph dev)
{
    if(current_impl_ == IMPL_TINYUSB)
    {
        // Store the callback function
        usb_rx_callback = cb;
        // Note: We ignore the dev parameter since TinyUSB uses the itf parameter
        // in the callback to distinguish between different CDC interfaces
        (void)dev;
    }
    else // IMPL_SIMPLE
    {
        // This is pretty silly, but we're working iteratively...
        rx_callback = cb;
        rxcallback  = (CDC_ReceiveCallback)rx_callback;

        switch(dev)
        {
            case FS_INTERNAL: CDC_Set_Rx_Callback_FS(rxcallback); break;
            case FS_EXTERNAL: CDC_Set_Rx_Callback_HS(rxcallback); break;
            case FS_BOTH:
                CDC_Set_Rx_Callback_FS(rxcallback);
                CDC_Set_Rx_Callback_HS(rxcallback);
                break;
            default: break;
        }
    }
}

size_t UsbHandle::GetRxAvailable() const
{
    if(current_impl_ == IMPL_TINYUSB)
    {
        return tud_cdc_available();
    }
    else // IMPL_SIMPLE
    {
        // Simple USB doesn't have a direct equivalent, return 0
        return 0;
    }
}

size_t UsbHandle::Receive(uint8_t *buff, size_t size)
{
    if(current_impl_ == IMPL_TINYUSB)
    {
        return tud_cdc_read(buff, size);
    }
    else // IMPL_SIMPLE
    {
        // Simple USB doesn't have a direct equivalent, return 0
        return 0;
    }
}

UsbHandle::Result UsbHandle::Flush()
{
    if(current_impl_ == IMPL_TINYUSB)
    {
        tud_cdc_write_flush();
        return Result::OK;
    }
    else // IMPL_SIMPLE
    {
        // Simple USB doesn't need explicit flushing
        return Result::OK;
    }
}

bool UsbHandle::IsTransmitReady(size_t size) const
{
    if(current_impl_ == IMPL_TINYUSB)
    {
        return tud_cdc_write_available() >= size;
    }
    else // IMPL_SIMPLE
    {
        // For simple USB, we'll assume always ready
        // You might want to implement a proper check here
        return true;
    }
}

bool UsbHandle::IsUsingTinyUsb() const
{
    return current_impl_ == IMPL_TINYUSB;
}

bool UsbHandle::IsConnected() const
{
    if(current_impl_ == IMPL_TINYUSB)
    {
        return tud_cdc_connected();
    }
    else // IMPL_SIMPLE
    {
        // For simple USB, we'll assume always connected
        // You might want to implement a proper check here
        return true;
    }
}

// Static Function Implementation
static void UsbErrorHandler()
{
    while(1) {}
}

// Helper function for system IRQ handlers
namespace daisy
{
UsbHandle::UsbImpl GetCurrentUsbImplementation()
{
    return UsbHandle::GetCurrentImplementation();
}
} // namespace daisy

// IRQ Handler - unified for both implementations
extern "C"
{
    // Shared USB IRQ Handlers for USB HS peripheral are located in sys/System.cpp

    void OTG_FS_EP1_OUT_IRQHandler(void)
    {
        HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
    }

    void OTG_FS_EP1_IN_IRQHandler(void)
    {
        HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
    }

    void OTG_FS_IRQHandler(void)
    {
        if(UsbHandle::GetCurrentImplementation() == UsbHandle::IMPL_TINYUSB)
        {
            tud_int_handler(BOARD_TUD_FS_RHPORT);
        }
        else // IMPL_SIMPLE
        {
            HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
        }
    }

    void tud_cdc_rx_cb(uint8_t itf)
    {
        // Only called for TinyUSB implementation
        if(usb_rx_callback == nullptr)
        {
            return;
        }

        // Loop as long as there's data available
        while(tud_cdc_available() > 0)
        {
            // Get the amount of data available for this iteration
            uint32_t count = tud_cdc_available();

            // Determine the size of the chunk to read
            uint32_t chunk_size = count;
            if(chunk_size > sizeof(rx_buffer))
            {
                chunk_size = sizeof(rx_buffer);
            }

            // Read the data into our buffer
            uint32_t read_bytes = tud_cdc_read(rx_buffer, chunk_size);

            // Call user callback with the data if bytes were read
            if(read_bytes > 0)
            {
                usb_rx_callback(rx_buffer, &read_bytes);
            }
        }
    }
}
