#include "hid/usb.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "tusb.h"

using namespace daisy;

static void UsbErrorHandler();

// Externs for IRQ Handlers
extern "C"
{
    // Globals from Cube generated version:
    USBD_HandleTypeDef       hUsbDeviceHS;
    USBD_HandleTypeDef       hUsbDeviceFS;
    extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
    extern PCD_HandleTypeDef hpcd_USB_OTG_HS;

    void DummyRxCallback(uint8_t* buf, uint32_t* size)
    {
        // Do Nothing
    }

    CDC_ReceiveCallback rxcallback;

    uint8_t usbd_mode = USBD_MODE_CDC;
}

UsbHandle::ReceiveCallback rx_callback;
uint8_t                    usb_fs_hw_initialized = 0;
uint8_t                    usb_hs_hw_initialized = 0;

static void InitFS()
{
    rx_callback = DummyRxCallback;
    // BUG: this can't be called multiple times. Need to have
    // a check to ensure it's not been called before.
    if(usb_fs_hw_initialized == 0)
    {
        usb_fs_hw_initialized = 1;
        if(USBD_Init(&hUsbDeviceFS, NULL, DEVICE_FS) != USBD_OK)
        {
            UsbErrorHandler();
        }
    }
    tud_init(BOARD_TUD_RHPORT);
    // if(USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK)
    // {
    //     UsbErrorHandler();
    // }
    // if(USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS)
    //    != USBD_OK)
    // {
    //     UsbErrorHandler();
    // }
    // if(USBD_Start(&hUsbDeviceFS) != USBD_OK)
    // {
    //     UsbErrorHandler();
    // }
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
    // HS as FS
    // if(USBD_Init(&hUsbDeviceHS, &HS_Desc, DEVICE_HS) != USBD_OK)
    // {
    //     UsbErrorHandler();
    // }
    // a check to ensure it's not been called before.
    rx_callback = DummyRxCallback;
    if(usb_hs_hw_initialized == 0)
    {
        usb_hs_hw_initialized = 1;
        if(USBD_Init(&hUsbDeviceHS, NULL, DEVICE_HS) != USBD_OK)
        {
            UsbErrorHandler();
        }
    }
    tud_init(1);


    // if(USBD_RegisterClass(&hUsbDeviceHS, &USBD_CDC) != USBD_OK)
    // {
    //     UsbErrorHandler();
    // }
    // if(USBD_CDC_RegisterInterface(&hUsbDeviceHS, &USBD_Interface_fops_HS)
    //    != USBD_OK)
    // {
    //     UsbErrorHandler();
    // }
    // if(USBD_Start(&hUsbDeviceHS) != USBD_OK)
    // {
    //     UsbErrorHandler();
    // }
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

UsbHandle::Result UsbHandle::TransmitInternal(uint8_t* buff, size_t size)
{
    // return CDC_Transmit_FS(buff, size) == USBD_OK ? Result::OK : Result::ERR;
    // auto ret = tud_cdc_write(buff, size) == size ? Result::OK : Result::ERR;
    // tud_cdc_write_flush();
    auto ret
        = tud_cdc_write(buff, size) == size ? Result::OK : Result::ERR;
    tud_cdc_write_flush();
    return ret;
}
UsbHandle::Result UsbHandle::TransmitExternal(uint8_t* buff, size_t size)
{
    auto ret
        = tud_cdc_write(buff, size) == size ? Result::OK : Result::ERR;
    tud_cdc_write_flush();
    return ret;

    // return CDC_Transmit_HS(buff, size) == USBD_OK ? Result::OK : Result::ERR;
}

void UsbHandle::SetReceiveCallback(ReceiveCallback cb, UsbPeriph dev)
{
    // This is pretty silly, but we're working iteritavely...
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

void UsbHandle::RunTask()
{
    tud_task();
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
        tud_int_handler(BOARD_TUD_RHPORT);
    }
}
