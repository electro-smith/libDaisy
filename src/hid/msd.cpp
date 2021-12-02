
#include "msd.h"
#include "daisy_core.h"
#include "usbh_core.h"
#include "usbh_msc.h"

using namespace daisy;

extern "C"
{
    extern HCD_HandleTypeDef                  hhcd_USB_OTG_HS;
    USBH_HandleTypeDef DMA_BUFFER_MEM_SECTION hUsbHostHS;
}

ApplicationTypeDef Appli_state = APPLICATION_IDLE;

class MSDHandle::Impl
{
  public:
    Impl() {}
    ~Impl() {}

    Result Init(Config config);
    Result Deinit();

    void Process();
    bool GetReady();

  private:
    Config config_;
};

// Global dfu handle
MSDHandle::Impl msd_impl;

static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);

MSDHandle::Result MSDHandle::Impl::Init(MSDHandle::Config config)
{
    config_ = config;
    /* Init host Library, add supported class and start the library. */
    if(USBH_Init(&hUsbHostHS, USBH_UserProcess, HOST_HS) != USBH_OK)
    {
        return Result::ERR;
    }
    if(USBH_RegisterClass(&hUsbHostHS, USBH_MSC_CLASS) != USBH_OK)
    {
        return Result::ERR;
    }
    if(USBH_Start(&hUsbHostHS) != USBH_OK)
    {
        return Result::ERR;
    }
    return Result::OK;
}

MSDHandle::Result MSDHandle::Impl::Deinit()
{
    USBH_Stop(&hUsbHostHS);
    USBH_DeInit(&hUsbHostHS);
    return Result::OK;
}

void MSDHandle::Impl::Process()
{
    USBH_Process(&hUsbHostHS);
}

bool MSDHandle::Impl::GetReady()
{
    return (bool)USBH_MSC_IsReady(&hUsbHostHS);
}

// MSDHandle -> Impl

MSDHandle::Result MSDHandle::Init(Config config)
{
    pimpl_ = &msd_impl;
    return pimpl_->Init(config);
}

MSDHandle::Result MSDHandle::Deinit()
{
    return pimpl_->Deinit();
}

bool MSDHandle::GetReady()
{
    return pimpl_->GetReady();
}

void MSDHandle::Process()
{
    pimpl_->Process();
}

bool MSDHandle::GetPresent()
{
    auto state = hUsbHostHS.gState;
    return (state != HOST_IDLE && state != HOST_ABORT_STATE
            && state != HOST_DEV_DISCONNECTED);
}

// IRQ Handler
extern "C"
{
    void OTG_HS_EP1_OUT_IRQHandler(void)
    {
        HAL_HCD_IRQHandler(&hhcd_USB_OTG_HS);
    }

    void OTG_HS_EP1_IN_IRQHandler(void)
    {
        HAL_HCD_IRQHandler(&hhcd_USB_OTG_HS);
    }

    void OTG_HS_IRQHandler(void) { HAL_HCD_IRQHandler(&hhcd_USB_OTG_HS); }
}

// This isn't super useful for our typical code structure
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
    switch(id)
    {
        case HOST_USER_SELECT_CONFIGURATION: break;

        case HOST_USER_DISCONNECTION:
            Appli_state = APPLICATION_DISCONNECT;
            break;

        case HOST_USER_CLASS_ACTIVE: Appli_state = APPLICATION_READY; break;

        case HOST_USER_CONNECTION: Appli_state = APPLICATION_START; break;

        default: break;
    }
}