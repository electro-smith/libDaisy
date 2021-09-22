
#include "msd.h"
#include "fatfs_usbh.h"
#include "usbh_core.h"
#include "usbh_msc.h"

using namespace daisy;

extern "C" 
{
  extern HCD_HandleTypeDef hhcd_USB_OTG_HS;
  USBH_HandleTypeDef hUsbHostHS;
}

ApplicationTypeDef Appli_state = APPLICATION_IDLE;

class MSDHandle::Impl {
  public:
    Impl() {}
    ~Impl() {}

    Result Init();
    Result Deinit(DaisySeed& hw);

    void Process();
    bool GetReady();

  private:

    DaisySeed* hw_;
        
};

// Global dfu handle
MSDHandle::Impl msd_impl;

static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);

MSDHandle::Result MSDHandle::Impl::Init()
{
  /* Init host Library, add supported class and start the library. */
  if (USBH_Init(&hUsbHostHS, USBH_UserProcess, HOST_HS) != USBH_OK)
  {
    return Result::ERR;
  }
  if (USBH_RegisterClass(&hUsbHostHS, USBH_MSC_CLASS) != USBH_OK)
  {
    return Result::ERR;
  }
  if (USBH_Start(&hUsbHostHS) != USBH_OK)
  {
    return Result::ERR;
  }
  MX_FATFS_Init();
  return Result::OK;
}

MSDHandle::Result MSDHandle::Impl::Deinit(DaisySeed& hw)
{
  USBH_Stop(&hUsbHostHS);
  USBH_DeInit(&hUsbHostHS);
  hw.DelayMs(100);
  return Result::OK;
}

void MSDHandle::Impl::Process()
{
  USBH_Process(&hUsbHostHS);
}

bool MSDHandle::Impl::GetReady()
{
  return (bool) USBH_MSC_IsReady(&hUsbHostHS);
}

// MSDHandle -> Impl

MSDHandle::Result MSDHandle::Init()
{
  pimpl_ = &msd_impl;
  return pimpl_->Init();
}

MSDHandle::Result MSDHandle::Deinit(DaisySeed& hw)
{
  return pimpl_->Deinit(hw);
}

bool MSDHandle::GetReady()
{
  return pimpl_->GetReady();
}

void MSDHandle::Process()
{
  pimpl_->Process();
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

    void OTG_HS_IRQHandler(void) 
    { 
        HAL_HCD_IRQHandler(&hhcd_USB_OTG_HS); 
    }
}

// This isn't super useful for our typical code structure
static void USBH_UserProcess (USBH_HandleTypeDef *phost, uint8_t id)
{
  switch(id)
  {
    case HOST_USER_SELECT_CONFIGURATION:
      break;

    case HOST_USER_DISCONNECTION:
      Appli_state = APPLICATION_DISCONNECT;
      break;

    case HOST_USER_CLASS_ACTIVE:
      Appli_state = APPLICATION_READY;
      break;

    case HOST_USER_CONNECTION:
      Appli_state = APPLICATION_START;
      break;

    default:
      break;
  }
}