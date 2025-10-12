/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 * Modified for Nopia/libDaisy SD Card MSC support
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "bsp/board_api.h"
#include "tusb.h"

#if CFG_TUD_MSC

// This implementation uses the SD card as the backing storage for MSC
// It interfaces with the STM32 SDMMC peripheral

// Include SD card HAL headers
#include "stm32h7xx_hal.h"

// SD card handle (should be defined elsewhere in libDaisy)
extern SD_HandleTypeDef hsd1;

// whether host does safe-eject
static bool ejected = false;

// SD card info
static HAL_SD_CardInfoTypeDef card_info;
static bool                   sd_initialized = false;

// Initialize SD card if not already done
static bool ensure_sd_ready(void)
{
    if(!sd_initialized)
    {
        // Get SD card information
        if(HAL_SD_GetCardInfo(&hsd1, &card_info) != HAL_OK)
        {
            return false;
        }

        // Check if SD card is ready
        if(HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
        {
            return false;
        }

        sd_initialized = true;
    }
    return true;
}

// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up to 8, 16, 4 characters respectively
void tud_msc_inquiry_cb(uint8_t lun,
                        uint8_t vendor_id[8],
                        uint8_t product_id[16],
                        uint8_t product_rev[4])
{
    (void)lun;

    const char vid[] = "Nopia";
    const char pid[] = "SD Card";
    const char rev[] = "1.0";

    memcpy(vendor_id, vid, strlen(vid));
    memcpy(product_id, pid, strlen(pid));
    memcpy(product_rev, rev, strlen(rev));
}

// Invoked when received Test Unit Ready command.
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    (void)lun;

    // Check if ejected
    if(ejected)
    {
        // Additional Sense 3A-00 is NOT_FOUND
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00);
        return false;
    }

    // Check if SD card is ready
    if(!ensure_sd_ready())
    {
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00);
        return false;
    }

    return true;
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY to determine the disk size
// Application update block count and block size
void tud_msc_capacity_cb(uint8_t   lun,
                         uint32_t *block_count,
                         uint16_t *block_size)
{
    (void)lun;

    if(ensure_sd_ready())
    {
        *block_count = card_info.LogBlockNbr;
        *block_size  = card_info.LogBlockSize;
    }
    else
    {
        *block_count = 0;
        *block_size  = 512; // Default block size
    }
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun,
                           uint8_t power_condition,
                           bool    start,
                           bool    load_eject)
{
    (void)lun;
    (void)power_condition;

    if(load_eject)
    {
        if(start)
        {
            // load disk storage
            ejected        = false;
            sd_initialized = false; // Force re-initialization
        }
        else
        {
            // unload disk storage
            ejected = true;
        }
    }

    return true;
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
int32_t tud_msc_read10_cb(uint8_t  lun,
                          uint32_t lba,
                          uint32_t offset,
                          void    *buffer,
                          uint32_t bufsize)
{
    (void)lun;

    if(!ensure_sd_ready())
    {
        return -1;
    }

    // Check for valid parameters
    if(lba >= card_info.LogBlockNbr)
    {
        return -1;
    }

    // For now, we only support reading full blocks (offset should be 0)
    // and bufsize should be a multiple of block size
    if(offset != 0 || (bufsize % card_info.LogBlockSize) != 0)
    {
        return -1;
    }

    uint32_t block_count = bufsize / card_info.LogBlockSize;

    // Read blocks from SD card
    if(HAL_SD_ReadBlocks(
           &hsd1, (uint8_t *)buffer, lba, block_count, HAL_MAX_DELAY)
       != HAL_OK)
    {
        return -1;
    }

    // Wait for transfer to complete
    while(HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
    {
        // Timeout could be added here
    }

    return (int32_t)bufsize;
}

bool tud_msc_is_writable_cb(uint8_t lun)
{
    (void)lun;

    // SD card is writable
    return true;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
int32_t tud_msc_write10_cb(uint8_t  lun,
                           uint32_t lba,
                           uint32_t offset,
                           uint8_t *buffer,
                           uint32_t bufsize)
{
    (void)lun;

    if(!ensure_sd_ready())
    {
        return -1;
    }

    // Check for valid parameters
    if(lba >= card_info.LogBlockNbr)
    {
        return -1;
    }

    // For now, we only support writing full blocks (offset should be 0)
    // and bufsize should be a multiple of block size
    if(offset != 0 || (bufsize % card_info.LogBlockSize) != 0)
    {
        return -1;
    }

    uint32_t block_count = bufsize / card_info.LogBlockSize;

    // Write blocks to SD card
    if(HAL_SD_WriteBlocks(&hsd1, buffer, lba, block_count, HAL_MAX_DELAY)
       != HAL_OK)
    {
        return -1;
    }

    // Wait for transfer to complete
    while(HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
    {
        // Timeout could be added here
    }

    return (int32_t)bufsize;
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
int32_t tud_msc_scsi_cb(uint8_t       lun,
                        uint8_t const scsi_cmd[16],
                        void         *buffer,
                        uint16_t      bufsize)
{
    void const *response = NULL;
    int32_t     resplen  = 0;

    // most scsi handled is input
    bool in_xfer = true;

    switch(scsi_cmd[0])
    {
        default:
            // Set Sense = Invalid Command Operation
            tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

            // negative means error -> tinyusb could stall and/or response with failed status
            resplen = -1;
            break;
    }

    // return resplen must not larger than bufsize
    if(resplen > bufsize)
        resplen = bufsize;

    if(response && (resplen > 0))
    {
        if(in_xfer)
        {
            memcpy(buffer, response, (size_t)resplen);
        }
        else
        {
            // SCSI output
        }
    }

    return (int32_t)resplen;
}

#endif // CFG_TUD_MSC
