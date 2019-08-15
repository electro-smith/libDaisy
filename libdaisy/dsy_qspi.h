/**
  ******************************************************************************
  * File Name          : dsy_qspi.h
  * Description        : This file provides code for the configuration
  *                      of the QUADSPI instances.
  ******************************************************************************
  * Currently supported Platforms:
  * - ES Daisy Rev3+
  * - 2hp Audio Brain Board Rev5+
  * 
  * Currently supported QSPI Devices
  * - IS25LP080D
  *
  ******************************************************************************
  */

#ifndef DSY_QSPI
#define DSY_QSPI

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

 //extern QSPI_HandleTypeDef hqspi;
 /* Error codes */
 #define MEMORY_OK          ((uint32_t)0x00)
 #define MEMORY_ERROR       ((uint32_t)0x01)
 enum
 {
	 DSY_QSPI_MODE_MEMORY_MAPPED,
	 DSY_QSPI_MODE_INDIRECT_POLLING,
	 DSY_QSPI_MODE_LAST,
 };

enum
{
	DSY_QSPI_DEVICE_IS25LP080D,
	DSY_QSPI_DEVICE_IS25LP064A,
	DSY_QSPI_DEVICE_LAST,
};

/*
 * @brief
 * Initializes QSPI peripheral, and Resets, and prepares memory for access.
 * @param dode
 * Mode describes the behavior of the interactions with the chip.
 * Writing is not permitted from Memory Mapped mode. 
 * @param device
 * Device specifies which Flash memory chip to use.
 */
 int dsy_qspi_init(uint8_t mode, uint8_t device);
 int dsy_qspi_deinit();

 /*
  *@brief 
  *Writes a single page to to the specified address on the QSPI chip.
  *For IS25LP080D page size is 256 bytes.
  */
 int dsy_qspi_writepage(uint32_t adr, uint32_t sz, uint8_t *buf);

 /*
  *@brief
  * Writes data to the flash memory. 
  */
int dsy_qspi_write(uint32_t address, uint32_t size, uint8_t* buffer);

 /*
  * @brief 
  * Erases the area specified on the chip. 
  * Erasures will happen by 4K, 32K or 64K increments.
  */
 int dsy_qspi_erase(uint32_t start_adr, uint32_t end_adr);

/*
 * @brief
 * Erases a single sector of the chip.
 * TODO: Document the size of this function.
 */
 int dsy_qspi_erasesector(uint32_t addr);


#ifdef __cplusplus
}
#endif

#endif