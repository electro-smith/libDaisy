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

// Add these near the other global declarations
static UsbHandle::ReceiveCallback usb_rx_callback = nullptr;
static uint8_t                    rx_buffer[CFG_TUD_CDC_RX_BUFSIZE];

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

UsbHandle::ReceiveCallback rx_callback;

static void InitFS()
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

static void DeinitFS()
{
    if(USBD_DeInit(&hUsbDeviceFS) != USBD_OK)
    {
        UsbErrorHandler();
    }
}

static void InitHS()
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

void UsbHandle::RunTask()
{
    // Protect against reentrant calls
    if(tud_task_running)
        return;

    tud_task_running = true;
    tud_task();
    tud_task_running = false;
}

static void DeinitHS()
{
    if(USBD_DeInit(&hUsbDeviceHS) != USBD_OK)
    {
        UsbErrorHandler();
    }
}

void UsbHandle::Init(UsbPeriph dev)
{
    switch(dev)
    {
        case FS_INTERNAL: InitFS(); break;
        case FS_EXTERNAL: InitHS(); break;
        case FS_BOTH:
            InitHS();
            InitFS();
            break;
        default: break;
    }
    // Enable USB Regulator
    HAL_PWREx_EnableUSBVoltageDetector();
}

void UsbHandle::DeInit(UsbPeriph dev)
{
    switch(dev)
    {
        case FS_INTERNAL: DeinitFS(); break;
        case FS_EXTERNAL: DeinitHS(); break;
        case FS_BOTH:
            DeinitHS();
            DeinitFS();
            break;
        default: break;
    }
    // Enable USB Regulator
    HAL_PWREx_DisableUSBVoltageDetector();
}

static UsbHandle::Result Transmit(uint8_t *buff, size_t size)
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
    return Transmit(buff, size);
}

UsbHandle::Result UsbHandle::TransmitExternal(uint8_t *buff, size_t size)
{
    return Transmit(buff, size);
}

void UsbHandle::SetReceiveCallback(ReceiveCallback cb, UsbPeriph dev)
{
    // Store the callback function
    usb_rx_callback = cb;

    // No need to do anything else, as TinyUSB will call tud_cdc_rx_cb()
    // whenever data is received, which will then call our callback

    // Note: We ignore the dev parameter since TinyUSB uses the itf parameter
    // in the callback to distinguish between different CDC interfaces
    (void)dev;
}

size_t UsbHandle::GetRxAvailable() const
{
    return tud_cdc_available();
}

size_t UsbHandle::Receive(uint8_t *buff, size_t size)
{
    // If using callbacks, we might want to buffer data differently
    // For now, we just pass through to tud_cdc_read
    return tud_cdc_read(buff, size);
}

UsbHandle::Result UsbHandle::Flush()
{
    tud_cdc_write_flush();
    return Result::OK;
}

bool UsbHandle::IsTransmitReady(size_t size) const
{
    return tud_cdc_write_available() >= size;
}

// Static Function Implementation
static void UsbErrorHandler()
{
    while(1) {}
}

// IRQ Handler
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
        tud_int_handler(BOARD_TUD_FS_RHPORT);
    }

    // Add this implementation of TinyUSB's callback function
    void tud_cdc_rx_cb(uint8_t itf)
    {
        (void)itf; // Unused parameter

        if(usb_rx_callback != nullptr)
        {
            // Get the amount of data available
            uint32_t count = tud_cdc_available();

            // Don't exceed buffer size
            if(count > sizeof(rx_buffer))
                count = sizeof(rx_buffer);

            // Only call the callback if there's data
            if(count > 0)
            {
                // Read the data into our buffer
                uint32_t read = tud_cdc_read(rx_buffer, count);

                // Call user callback with the data
                usb_rx_callback(rx_buffer, &read);
            }
        }
    }
}
