/**
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                      www.st.com/SLA0044
 *
 ******************************************************************************
 */

#include "usbh_midi.h"
#include "daisy_core.h"

static MIDI_HandleTypeDef DMA_BUFFER_MEM_SECTION static_midi;

static USBH_StatusTypeDef USBH_MIDI_InterfaceInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_MIDI_InterfaceDeInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_MIDI_Process(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_MIDI_ClassRequest(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_MIDI_SOFProcess(USBH_HandleTypeDef *phost);

USBH_ClassTypeDef USBH_midi = {
    "MIDI",
    USB_AUDIO_CLASS,
    USBH_MIDI_InterfaceInit,
    USBH_MIDI_InterfaceDeInit,
    USBH_MIDI_ClassRequest,
    USBH_MIDI_Process,
    USBH_MIDI_SOFProcess,
    NULL,
};

#define EP_IN 0x80U

/**
 * @brief  USBH_MIDI_InterfaceInit
 *         The function init the MIDI class.
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_MIDI_InterfaceInit(USBH_HandleTypeDef *phost)
{
    USBH_StatusTypeDef status;
    MIDI_HandleTypeDef *MIDI_Handle;
    uint8_t interface = USBH_FindInterface(phost, phost->pActiveClass->ClassCode,
            USB_MIDI_STREAMING_SUBCLASS, 0xFFU);

    if ((interface == 0xFFU) || (interface >= USBH_MAX_NUM_INTERFACES)) {
        USBH_DbgLog("Cannot Find the interface for %s class.", phost->pActiveClass->Name);
        return USBH_FAIL;
    }

    status = USBH_SelectInterface(phost, interface);
    if (status != USBH_OK) {
        return USBH_FAIL;
    }

    // Single static instance of midi handle
    phost->pActiveClass->pData = &static_midi;
    MIDI_Handle = (MIDI_HandleTypeDef*)phost->pActiveClass->pData;

    /* Initialize midi handler */
    USBH_memset(MIDI_Handle, 0, sizeof(MIDI_HandleTypeDef));

    /* Find the endpoints */
    for (int ep = 0; ep <= 1; ++ep) {
        if (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[ep].bEndpointAddress & EP_IN) {
            MIDI_Handle->InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[ep].bEndpointAddress;
            MIDI_Handle->InEpSize = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[ep].wMaxPacketSize;
        } else {
            MIDI_Handle->OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[ep].bEndpointAddress;
            MIDI_Handle->OutEpSize = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[ep].wMaxPacketSize;
        }
    }

    MIDI_Handle->state = MIDI_INIT;
    MIDI_Handle->error = MIDI_OK;
    MIDI_Handle->InPipe = USBH_AllocPipe(phost, MIDI_Handle->InEp);

    /* Open input pipe */
    USBH_OpenPipe(phost, MIDI_Handle->InPipe, MIDI_Handle->InEp,
            phost->device.address, phost->device.speed, USB_EP_TYPE_BULK,
            MIDI_Handle->InEpSize);

    USBH_LL_SetToggle(phost, MIDI_Handle->InPipe, 0U);

    /* Open input pipe */
    USBH_OpenPipe(phost, MIDI_Handle->OutPipe, MIDI_Handle->OutEp,
            phost->device.address, phost->device.speed, USB_EP_TYPE_BULK,
            MIDI_Handle->OutEpSize);

    USBH_LL_SetToggle(phost, MIDI_Handle->OutPipe, 0U);

    return USBH_OK;
}

/**
 * @brief  USBH_MIDI_InterfaceDeInit
 *         The function DeInit the Pipes used for the MIDI class.
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_MIDI_InterfaceDeInit(USBH_HandleTypeDef *phost)
{
    MIDI_HandleTypeDef *MIDI_Handle = (MIDI_HandleTypeDef *) phost->pActiveClass->pData;
    if (MIDI_Handle->InPipe) {
        USBH_ClosePipe(phost, MIDI_Handle->InPipe);
        USBH_FreePipe(phost, MIDI_Handle->InPipe);
        MIDI_Handle->InPipe = 0U;     /* Reset the Channel as Free */
    }
    if (phost->pActiveClass->pData) {
        phost->pActiveClass->pData = 0U;
    }
    return USBH_OK;
}

/**
 * @brief  USBH_MIDI_ClassRequest
 *         The function is responsible for handling Standard requests
 *         for MIDI class.
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_MIDI_ClassRequest(USBH_HandleTypeDef *phost)
{
    phost->pUser(phost, HOST_USER_CLASS_ACTIVE);
    return USBH_OK;
}

/**
 * @brief  USBH_MIDI_Process
 *         The function is for managing state machine for MIDI data transfers
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_MIDI_Process(USBH_HandleTypeDef *phost)
{
    MIDI_HandleTypeDef *hMidi = (MIDI_HandleTypeDef*)phost->pActiveClass->pData;
    USBH_StatusTypeDef error = USBH_OK;
    USBH_URBStateTypeDef rxStatus;

    switch (hMidi->state) {
        case MIDI_INIT:
            hMidi->state = MIDI_IDLE;
            break;
        case MIDI_IDLE:
            USBH_BulkReceiveData(phost, hMidi->rxBuffer, USBH_MIDI_RX_BUF_SIZE, hMidi->InPipe);
            hMidi->state = MIDI_RX;
            break;
        case MIDI_RX:
            rxStatus = USBH_LL_GetURBState(phost, hMidi->InPipe);
            if (rxStatus == USBH_URB_DONE) {
                size_t sz = USBH_LL_GetLastXferSize(phost, hMidi->InPipe);
                hMidi->state = MIDI_IDLE;
                hMidi->callback(hMidi->rxBuffer, sz, hMidi->pUser);
            } else if (rxStatus == USBH_URB_ERROR || rxStatus == USBH_URB_STALL) {
                hMidi->state = MIDI_FAIL;
                error = USBH_FAIL;
            }
            break;
        case MIDI_FAIL:
            error = USBH_ClrFeature(phost, 0);
            if (error == USBH_OK) {
                hMidi->state = MIDI_IDLE;
            }
            break;
    }
    return error;
}

/**
 * @brief  USBH_MIDI_SOFProcess
 *         The function is for SOF state
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_MIDI_SOFProcess(USBH_HandleTypeDef *phost)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(phost);
    return USBH_OK;
}

void USBH_MIDI_SetReceiveCallback(USBH_HandleTypeDef *phost, USBH_MIDI_RxCallback cb, void* pUser)
{
    MIDI_HandleTypeDef *hMidi = (MIDI_HandleTypeDef*)phost->pActiveClass->pData;
    hMidi->callback = cb;
    hMidi->pUser = pUser;
}

MIDI_ErrorTypeDef USBH_MIDI_Transmit(USBH_HandleTypeDef *phost, uint8_t* data, size_t len)
{
    MIDI_HandleTypeDef *hMidi = (MIDI_HandleTypeDef*)phost->pActiveClass->pData;
    // TODO - This is currently always a blocking write
    while (len) {
        size_t sz = (len <= hMidi->OutEpSize) ? len : hMidi->OutEpSize;
        USBH_BulkSendData(phost, data, sz, hMidi->OutPipe, 1);
        USBH_URBStateTypeDef txStatus;
        do {
            txStatus = USBH_LL_GetURBState(phost, hMidi->OutPipe);
            if (txStatus == USBH_URB_ERROR || txStatus == USBH_URB_STALL) {
                return MIDI_ERROR;
            }
        } while (txStatus != USBH_URB_DONE);
        len -= sz;
    }
    return MIDI_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
