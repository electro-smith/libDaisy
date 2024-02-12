
#include "usb_host.h"
#include "daisy_core.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "logger.h"

using namespace daisy;

extern "C"
{
    USBH_HandleTypeDef DMA_BUFFER_MEM_SECTION hUsbHostHS;
    void USBH_LogPrint(const char* format, ...);
}

void USBH_LogPrint(const char* format, ...)
{
    va_list va;
    va_start(va, format);
    Logger<LOGGER_INTERNAL>::PrintLineV(format, va);
    va_end(va);
}

ApplicationTypeDef Appli_state = APPLICATION_IDLE;

class USBHostHandle::Impl
{
    public:
        Impl() {
            memset(&hUsbHostHS, 0, sizeof(hUsbHostHS));
        }
        ~Impl() {}

        Result Init(Config config, USBH_ClassTypeDef* pClass);
        Result Deinit();
        Result Process();
        Result ReEnumerate();

        bool GetReady();

        inline Config& GetConfig(USBH_ClassTypeDef* pClass) {
            for (uint32_t i = 0; i < hUsbHostHS.ClassNumber; ++i) {
                if(hUsbHostHS.pClass[i] == pClass)
                    return config_[i];
            }
            return config_[0];
        }

    private:
        Config config_[USBH_MAX_NUM_SUPPORTED_CLASS];

        /** @brief Maps ST Middleware USBH_StatusTypeDef to USBHostHandle::Result codes */
        Result ConvertStatus(USBH_StatusTypeDef sta)
        {
            if(sta != USBH_OK)
            {
                return Result::FAIL;
            }
            switch(sta)
            {
                case USBH_OK: return Result::OK;
                case USBH_BUSY: return Result::BUSY;
                case USBH_NOT_SUPPORTED: return Result::NOT_SUPPORTED;
                case USBH_UNRECOVERED_ERROR: return Result::UNRECOVERED_ERROR;
                case USBH_ERROR_SPEED_UNKNOWN: return Result::ERROR_SPEED_UNKNOWN;
                case USBH_FAIL:
                default: return Result::FAIL;
            }
        }
};

// Global handle
static USBHostHandle::Impl usbh_impl;

static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);

USBHostHandle::Result USBHostHandle::Impl::Init(USBHostHandle::Config config, USBH_ClassTypeDef* pClass)
{
    USBH_UsrLog("USBHostHandle:Init %d", hUsbHostHS.ClassNumber);
    /* Init host Library, add supported class and start the library. */
    USBH_StatusTypeDef sta;
    int classNumber = hUsbHostHS.ClassNumber;
    // Initialize USB Host driver for first registered class
    if(classNumber == 0)
    {
        sta = USBH_Init(&hUsbHostHS, USBH_UserProcess, HOST_HS);
        if(sta != USBH_OK)
        {
            return ConvertStatus(sta);
        }
    }
    sta = USBH_RegisterClass(&hUsbHostHS, pClass);
    if(sta != USBH_OK)
    {
        return ConvertStatus(sta);
    }
    if(classNumber == 0)
    {
        sta = USBH_Start(&hUsbHostHS);
        if(sta != USBH_OK)
        {
            return ConvertStatus(sta);
        }
    }
    config_[classNumber] = config;
    return ConvertStatus(sta);
}

USBHostHandle::Result USBHostHandle::Impl::Deinit()
{
    USBH_Stop(&hUsbHostHS);
    USBH_DeInit(&hUsbHostHS);
    return Result::OK;
}

USBHostHandle::Result USBHostHandle::Impl::Process()
{
    return ConvertStatus(USBH_Process(&hUsbHostHS));
}

USBHostHandle::Result USBHostHandle::Impl::ReEnumerate()
{
    return ConvertStatus(USBH_ReEnumerate(&hUsbHostHS));
}

bool USBHostHandle::Impl::GetReady()
{
    return Appli_state == APPLICATION_READY;
}

USBHostHandle::Result USBHostHandle::Init(Config config, USBH_ClassTypeDef* pClass)
{
    pimpl_ = &usbh_impl;
    return pimpl_->Init(config, pClass);
}

USBHostHandle::Result USBHostHandle::Deinit()
{
    return pimpl_->Deinit();
}

bool USBHostHandle::GetReady()
{
    return pimpl_->GetReady();
}

USBHostHandle::Result USBHostHandle::Process()
{
    return pimpl_->Process();
}

USBHostHandle::Result USBHostHandle::ReEnumerate()
{
    return pimpl_->ReEnumerate();
}

bool USBHostHandle::IsActiveClass(USBH_ClassTypeDef* pClass)
{
    return pClass == hUsbHostHS.pActiveClass;
}

bool USBHostHandle::GetPresent()
{
    auto state = hUsbHostHS.gState;
    return (state != HOST_IDLE && state != HOST_ABORT_STATE
            && state != HOST_DEV_DISCONNECTED);
}

// Shared USB IRQ Handlers are located in sys/System.cpp

// This isn't super useful for our typical code structure
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
    USBH_UsrLog("USBH_UserProcess %d", id);
    auto &conf = usbh_impl.GetConfig(phost->pActiveClass);
    switch(id)
    {
        case HOST_USER_SELECT_CONFIGURATION:
            break;
        case HOST_USER_CLASS_ACTIVE:
            Appli_state = APPLICATION_READY;
            if(conf.class_active_callback)
            {
                auto cb = (conf.class_active_callback);
                cb(conf.userdata);
            }
            break;
        case HOST_USER_CLASS_SELECTED:
            break;
        case HOST_USER_CONNECTION:
            Appli_state = APPLICATION_START;
            if(conf.connect_callback)
            {
                auto cb = (conf.connect_callback);
                cb(conf.userdata);
            }
            break;
        case HOST_USER_DISCONNECTION:
            Appli_state = APPLICATION_DISCONNECT;
            if(conf.disconnect_callback)
            {
                auto cb = (conf.disconnect_callback);
                cb(conf.userdata);
            }
            break;
        case HOST_USER_UNRECOVERED_ERROR:
            if(conf.error_callback)
            {
                auto cb = (conf.error_callback);
                cb(conf.userdata);
            }
            break;
        default:
            break;
    }
}
