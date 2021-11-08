/**
  ******************************************************************************
  * @file    usbh_diskio.h (based on usbh_diskio_dma_template.h v2.0.2)
  * @brief   Header for usbh_diskio.c module
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
#ifndef __USBH_DISKIO_H
#define __USBH_DISKIO_H

/* USER CODE BEGIN firstSection */
/* can be used to modify / undefine following code or add new definitions */
/* USER CODE END firstSection */

/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"
#include "usbh_msc.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern const Diskio_drvTypeDef USBH_Driver;

/* USER CODE BEGIN lastSection */
/* can be used to modify / undefine previous code or add new definitions */
/* USER CODE END lastSection */

#endif /* __USBH_DISKIO_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
