# USB Device Library
add_library(STM32_USB_DEVICE_LIBRARY STATIC
  ../Patched/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c
  STM32_USB_Device_Library/Core/Src/usbd_core.c
  STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c
  STM32_USB_Device_Library/Core/Src/usbd_ioreq.c
)
target_include_directories(STM32_USB_DEVICE_LIBRARY PUBLIC
  STM32_USB_Device_Library/Core/Inc
  ../Patched/ST/STM32_USB_Device_Library/Class/CDC/Inc
  ${MODULE_DIR}/usbd # for conf
)
target_link_libraries(STM32_USB_DEVICE_LIBRARY PUBLIC
  CMSIS_DEVICE_H7
  STM32H7XX_HAL_DRIVER
)

# USB Host Library
add_library(STM32_USB_HOST_LIBRARY STATIC
  STM32_USB_Host_Library/Core/Src/usbh_core.c
  STM32_USB_Host_Library/Core/Src/usbh_ctlreq.c
  STM32_USB_Host_Library/Core/Src/usbh_ioreq.c
  STM32_USB_Host_Library/Core/Src/usbh_pipes.c
  STM32_USB_Host_Library/Class/MSC/Src/usbh_msc_bot.c
  STM32_USB_Host_Library/Class/MSC/Src/usbh_msc_scsi.c
  STM32_USB_Host_Library/Class/MSC/Src/usbh_msc.c
  STM32_USB_Host_Library/Class/MIDI/Src/usbh_midi.c
)
target_include_directories(STM32_USB_HOST_LIBRARY PUBLIC
  STM32_USB_Host_Library/Core/Inc
  STM32_USB_Host_Library/Class/MSC/Inc
  STM32_USB_Host_Library/Class/MIDI/Inc
  ${MODULE_DIR}/usbh # for conf
  ${MODULE_DIR} # for daisy_core.h
)
target_link_libraries(STM32_USB_HOST_LIBRARY PUBLIC
  CMSIS_DEVICE_H7
  STM32H7XX_HAL_DRIVER
)
