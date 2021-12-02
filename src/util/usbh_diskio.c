/**
  ******************************************************************************
  * @file    usbh_diskio.c (based on usbh_diskio_dma_template.c v2.0.2)
  * @brief   USB Host Disk I/O driver
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
/* USER CODE BEGIN firstSection */
/* can be used to modify / undefine following code or add new definitions */

/* USER CODE END firstSection */

/* Includes ------------------------------------------------------------------*/
#include "ff_gen_drv.h"
#include "usbh_diskio.h"
#include "daisy_core.h"

extern USBH_HandleTypeDef hUsbHostHS;
#define hUSB_Host hUsbHostHS

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define USB_DEFAULT_BLOCK_SIZE 512

#define ENABLE_USB_DMA_CACHE_MAINTENANCE 1

/* Private variables ---------------------------------------------------------*/
static DWORD DMA_BUFFER_MEM_SECTION scratch[_MAX_SS / 4];
extern USBH_HandleTypeDef           hUSB_Host;

/* Private function prototypes -----------------------------------------------*/
DSTATUS USBH_initialize(BYTE);
DSTATUS USBH_status(BYTE);
DRESULT USBH_read(BYTE, BYTE *, DWORD, UINT);

#if _USE_WRITE == 1
DRESULT USBH_write(BYTE, const BYTE *, DWORD, UINT);
#endif /* _USE_WRITE == 1 */

#if _USE_IOCTL == 1
DRESULT USBH_ioctl(BYTE, BYTE, void *);
#endif /* _USE_IOCTL == 1 */

const Diskio_drvTypeDef USBH_Driver = {
    USBH_initialize,
    USBH_status,
    USBH_read,
#if _USE_WRITE == 1
    USBH_write,
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
    USBH_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* USER CODE BEGIN beforeFunctionSection */
/* can be used to modify / undefine following code or add new code */
/* USER CODE END beforeFunctionSection */

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  lun : lun id
  * @retval DSTATUS: Operation status
  */
DSTATUS USBH_initialize(BYTE lun)
{
    /* CAUTION : USB Host library has to be initialized in the application */

    return RES_OK;
}

/**
  * @brief  Gets Disk Status
  * @param  lun : lun id
  * @retval DSTATUS: Operation status
  */
DSTATUS USBH_status(BYTE lun)
{
    DRESULT res = RES_ERROR;

    if(USBH_MSC_UnitIsReady(&hUSB_Host, lun))
    {
        res = RES_OK;
    }
    else
    {
        res = RES_ERROR;
    }

    return res;
}

/* USER CODE BEGIN beforeReadSection */
/* can be used to modify previous code / undefine following code / add new code */
/* USER CODE END beforeReadSection */

/**
  * @brief  Reads Sector(s)
  * @param  lun : lun id
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT USBH_read(BYTE lun, BYTE *buff, DWORD sector, UINT count)
{
    DRESULT            res = RES_ERROR;
    MSC_LUNTypeDef     info;
    USBH_StatusTypeDef status = USBH_OK;

#if(ENABLE_USB_DMA_CACHE_MAINTENANCE == 1)
    uint32_t alignedAddr;
    alignedAddr = (uint32_t)buff & ~0x1F;
    SCB_CleanDCache_by_Addr((uint32_t *)alignedAddr,
                            count * BLOCKSIZE + ((uint32_t)buff - alignedAddr));
#endif

    if(((DWORD)buff & 3)
       && (((HCD_HandleTypeDef *)hUSB_Host.pData)->Init.dma_enable))
    {
        while((count--) && (status == USBH_OK))
        {
            status = USBH_MSC_Read(
                &hUSB_Host, lun, sector + count, (uint8_t *)scratch, 1);

            if(status == USBH_OK)
            {
                memcpy(&buff[count * _MAX_SS], scratch, _MAX_SS);
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        status = USBH_MSC_Read(&hUSB_Host, lun, sector, buff, count);
    }
#if(ENABLE_USB_DMA_CACHE_MAINTENANCE == 1)
    /* the SCB_InvalidateDCache_by_Addr() requires a 32-Byte aligned address,
                     * adjust the address and the D-Cache size to invalidate accordingly. */
    SCB_InvalidateDCache_by_Addr((uint32_t *)alignedAddr,
                                 count * BLOCKSIZE
                                     + ((uint32_t)buff - alignedAddr));
#endif

    if(status == USBH_OK)
    {
        res = RES_OK;
    }
    else
    {
        USBH_MSC_GetLUNInfo(&hUSB_Host, lun, &info);

        switch(info.sense.asc)
        {
            case SCSI_ASC_LOGICAL_UNIT_NOT_READY:
            case SCSI_ASC_MEDIUM_NOT_PRESENT:
            case SCSI_ASC_NOT_READY_TO_READY_CHANGE:
                USBH_ErrLog("USB Disk is not ready!");
                res = RES_NOTRDY;
                break;

            default: res = RES_ERROR; break;
        }
    }

    return res;
}

/* USER CODE BEGIN beforeWriteSection */
/* can be used to modify previous code / undefine following code / add new code */
/* USER CODE END beforeWriteSection */

/**
  * @brief  Writes Sector(s)
  * @param  lun : lun id
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT USBH_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
    DRESULT            res = RES_ERROR;
    MSC_LUNTypeDef     info;
    USBH_StatusTypeDef status = USBH_OK;
#if(ENABLE_USB_DMA_CACHE_MAINTENANCE == 1)
    uint32_t alignedAddr;
    /*
    the SCB_CleanDCache_by_Addr() requires a 32-Byte aligned address
    adjust the address and the D-Cache size to clean accordingly.
    */
    alignedAddr = (uint32_t)buff & ~0x1F;
    SCB_CleanDCache_by_Addr((uint32_t *)alignedAddr,
                            count * BLOCKSIZE + ((uint32_t)buff - alignedAddr));
#endif

    if(((DWORD)buff & 3)
       && (((HCD_HandleTypeDef *)hUSB_Host.pData)->Init.dma_enable))
    {
        while(count--)
        {
            memcpy(scratch, &buff[count * _MAX_SS], _MAX_SS);

            status = USBH_MSC_Write(
                &hUSB_Host, lun, sector + count, (BYTE *)scratch, 1);
            if(status == USBH_FAIL)
            {
                break;
            }
        }
    }
    else
    {
        status = USBH_MSC_Write(&hUSB_Host, lun, sector, (BYTE *)buff, count);
    }

    if(status == USBH_OK)
    {
        res = RES_OK;
    }
    else
    {
        USBH_MSC_GetLUNInfo(&hUSB_Host, lun, &info);

        switch(info.sense.asc)
        {
            case SCSI_ASC_WRITE_PROTECTED:
                USBH_ErrLog("USB Disk is Write protected!");
                res = RES_WRPRT;
                break;

            case SCSI_ASC_LOGICAL_UNIT_NOT_READY:
            case SCSI_ASC_MEDIUM_NOT_PRESENT:
            case SCSI_ASC_NOT_READY_TO_READY_CHANGE:
                USBH_ErrLog("USB Disk is not ready!");
                res = RES_NOTRDY;
                break;

            default: res = RES_ERROR; break;
        }
    }

    return res;
}
#endif /* _USE_WRITE == 1 */

/* USER CODE BEGIN beforeIoctlSection */
/* can be used to modify previous code / undefine following code / add new code */
/* USER CODE END beforeIoctlSection */

/**
  * @brief  I/O control operation
  * @param  lun : lun id
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT USBH_ioctl(BYTE lun, BYTE cmd, void *buff)
{
    DRESULT        res = RES_ERROR;
    MSC_LUNTypeDef info;

    switch(cmd)
    {
        /* Make sure that no pending write process */
        case CTRL_SYNC: res = RES_OK; break;

        /* Get number of sectors on the disk (DWORD) */
        case GET_SECTOR_COUNT:
            if(USBH_MSC_GetLUNInfo(&hUSB_Host, lun, &info) == USBH_OK)
            {
                *(DWORD *)buff = info.capacity.block_nbr;
                res            = RES_OK;
            }
            else
            {
                res = RES_ERROR;
            }
            break;

        /* Get R/W sector size (WORD) */
        case GET_SECTOR_SIZE:
            if(USBH_MSC_GetLUNInfo(&hUSB_Host, lun, &info) == USBH_OK)
            {
                *(DWORD *)buff = info.capacity.block_size;
                res            = RES_OK;
            }
            else
            {
                res = RES_ERROR;
            }
            break;

            /* Get erase block size in unit of sector (DWORD) */
        case GET_BLOCK_SIZE:

            if(USBH_MSC_GetLUNInfo(&hUSB_Host, lun, &info) == USBH_OK)
            {
                *(DWORD *)buff
                    = info.capacity.block_size / USB_DEFAULT_BLOCK_SIZE;
                res = RES_OK;
            }
            else
            {
                res = RES_ERROR;
            }
            break;

        default: res = RES_PARERR;
    }

    return res;
}
#endif /* _USE_IOCTL == 1 */

/* USER CODE BEGIN lastSection */
/* can be used to modify / undefine previous code or add new code */
/* USER CODE END lastSection */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
