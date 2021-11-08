/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
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

#include "fatfs_usbh.h"
#include "daisy_core.h"

uint8_t retUSBH;     /* Return value for USBH */
char    USBHPath[4]; /* USBH logical drive path */

// NOTE -- these _must_ be in a cacheless section to permit DMA transfers
FATFS DMA_BUFFER_MEM_SECTION
                           USBHFatFS; /* File system object for USBH logical drive */
FIL DMA_BUFFER_MEM_SECTION USBHFile; /* File object for USBH */

/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
    /*## FatFS: Link the USBH driver ###########################*/
    retUSBH = FATFS_LinkDriver(&USBH_Driver, USBHPath);

    /* USER CODE BEGIN Init */
    /* additional user code for init */
    /* USER CODE END Init */
}

/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
