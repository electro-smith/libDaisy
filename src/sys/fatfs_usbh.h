/**
  ******************************************************************************
  * @file   fatfs.h
  * @brief  Header for fatfs applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __fatfsusb_H
#define __fatfsusb_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "ff.h"
#include "ff_gen_drv.h"
#include "util/usbh_diskio.h" /* defines USBH_Driver as external */

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern uint8_t retUSBH; /* Return value for USBH */
extern char USBHPath[4]; /* USBH logical drive path */
extern FATFS USBHFatFS; /* File system object for USBH logical drive */
extern FIL USBHFile; /* File object for USBH */

void MX_FATFS_Init(void);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */
#ifdef __cplusplus
}
#endif
#endif /*__fatfs_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
