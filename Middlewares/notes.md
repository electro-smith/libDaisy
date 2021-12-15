# Middleware Notes

This can be used to keep track of any manual changes to Middleware files to make migrating to new versions easier in the future.

* Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Src/usbh_msc.c
  * The msc class was modified to prevent dynamic allocation of the `MSC_HandleTypeDef` struct. It was also placed in uncached D2 ram to allow DMA transfers with D cache enabled.