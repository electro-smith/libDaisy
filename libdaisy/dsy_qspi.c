#include "dsy_qspi.h"
#include "stm32h7xx_hal.h"
#include "IS25LP080D.h"

// TODO: Add handling for alternate device types,
//		This will be a thing much sooner than anticipated 
//		due to upgrading the RAM size for the new 4MB chip.
// TODO: autopolling_mem_ready only works for 1-Line, not 4-Line

//typedef struct 
//{
//	QSPI_HandleTypeDef hqspi;	
//	uint8_t quad_mode;
//}dsy_qspi_t;
static uint32_t reset_memory(QSPI_HandleTypeDef *hqspi);
static uint32_t dummy_cycles_cfg(QSPI_HandleTypeDef *hqspi);
static uint32_t write_enable(QSPI_HandleTypeDef *hqspi);
static uint32_t quad_enable(QSPI_HandleTypeDef *hqspi);
static uint32_t enable_memory_mapped_mode(QSPI_HandleTypeDef *hqspi);
static uint32_t autopolling_mem_ready(QSPI_HandleTypeDef *hqspi, uint32_t timeout);
static uint32_t enter_quad_mode(QSPI_HandleTypeDef *hqspi);
static uint32_t exit_quad_mode(QSPI_HandleTypeDef *hqspi);
static uint8_t get_status_register(QSPI_HandleTypeDef *hqspi);


static QSPI_HandleTypeDef dsy_qspi_handle;

int dsy_qspi_init(uint8_t mode, uint8_t device)
{
	// Set Handle Settings 	o
	if(HAL_QSPI_DeInit(&dsy_qspi_handle) != HAL_OK)
	{
		return MEMORY_ERROR;
	}
	//HAL_QSPI_MspInit(&dsy_qspi_handle);	 // I think this gets called a in HAL_QSPI_Init();
	// Set Initialization values for the QSPI Peripheral
	uint32_t flash_size;
	switch(device)
	{
	case DSY_QSPI_DEVICE_IS25LP080D:
		flash_size = IS25LP080D_FLASH_SIZE;
		break;
	default:
		flash_size = IS25LP080D_FLASH_SIZE;
		break;
	}
	dsy_qspi_handle.Instance = QUADSPI;
	dsy_qspi_handle.Init.ClockPrescaler = 10;
	dsy_qspi_handle.Init.FifoThreshold = 1;
	dsy_qspi_handle.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
	dsy_qspi_handle.Init.FlashSize = POSITION_VAL(IS25LP080D_FLASH_SIZE) - 1;
	dsy_qspi_handle.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
	dsy_qspi_handle.Init.FlashID = QSPI_FLASH_ID_1;
	dsy_qspi_handle.Init.DualFlash = QSPI_DUALFLASH_DISABLE;

	if (HAL_QSPI_Init(&dsy_qspi_handle) != HAL_OK)
	{
		return MEMORY_ERROR;
	}
	if (reset_memory(&dsy_qspi_handle) != MEMORY_OK)
	{
		return MEMORY_ERROR;	
	}
	uint8_t fifothresh = HAL_QSPI_GetFifoThreshold(&dsy_qspi_handle);
//	uint8_t reg = 0;
//	reg = get_status_register(&dsy_qspi_handle);
	if (dummy_cycles_cfg(&dsy_qspi_handle) != MEMORY_OK)
	{
		return MEMORY_ERROR;
	}
	if (quad_enable(&dsy_qspi_handle) != MEMORY_OK)
	{
		return MEMORY_ERROR;
	}
	if (mode == DSY_QSPI_MODE_MEMORY_MAPPED)
	{
		if (enable_memory_mapped_mode(&dsy_qspi_handle) != MEMORY_OK)
		{
			return MEMORY_ERROR;
		}
	}
	return MEMORY_OK;
}

int dsy_qspi_writepage(uint32_t adr, uint32_t sz, uint8_t *buf)
{
	QSPI_CommandTypeDef s_command;
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = PAGE_PROG_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
	s_command.AddressSize		= QSPI_ADDRESS_24_BITS;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_1_LINE;
	s_command.DummyCycles       = 0;
	s_command.NbData            = sz <= 256 ? sz : 256;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
	s_command.Address = adr;
	if (write_enable(&dsy_qspi_handle) != MEMORY_OK)
	{
		return MEMORY_ERROR;
	}
	if (HAL_QSPI_Command(&dsy_qspi_handle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != MEMORY_OK)
	{
		return MEMORY_ERROR;
	}
	if (HAL_QSPI_Transmit(&dsy_qspi_handle, (uint8_t*)buf, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != MEMORY_OK)
	{
		return MEMORY_ERROR;
	}
	if (autopolling_mem_ready(&dsy_qspi_handle, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != MEMORY_OK)
	{
		return MEMORY_ERROR;
	}
	return MEMORY_OK;
}

int dsy_qspi_write(uint32_t address, uint32_t size, uint8_t* buffer)
{
	uint32_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
	uint32_t   QSPI_DataNum = 0;
	uint32_t flash_page_size = IS25LP080D_PAGE_SIZE; 
	Addr = address % flash_page_size;
	count = flash_page_size - Addr;
	NumOfPage =  size / flash_page_size;
	NumOfSingle = size % flash_page_size;
  
	if (Addr == 0) /*!< Address is QSPI_PAGESIZE aligned  */
	{
		if (NumOfPage == 0) /*!< NumByteToWrite < QSPI_PAGESIZE */
		{
			QSPI_DataNum = size;      
			dsy_qspi_writepage(address, QSPI_DataNum, buffer);
		}
		else /*!< Size > QSPI_PAGESIZE */
		{
			while (NumOfPage--)
			{
				QSPI_DataNum = flash_page_size;
				dsy_qspi_writepage(address, QSPI_DataNum, buffer);
				address +=  flash_page_size;
				buffer += flash_page_size;
			}
      
			QSPI_DataNum = NumOfSingle;
			if (QSPI_DataNum > 0)
				dsy_qspi_writepage(address, QSPI_DataNum, buffer);
		}
	}
	else /*!< Address is not QSPI_PAGESIZE aligned  */
	{
		if (NumOfPage == 0) /*!< Size < QSPI_PAGESIZE */
		{
			if (NumOfSingle > count) /*!< (Size + Address) > QSPI_PAGESIZE */
			{
				temp = NumOfSingle - count;
				QSPI_DataNum = count;
				dsy_qspi_writepage(address, QSPI_DataNum, buffer);
				address +=  count;
				buffer += count;
				QSPI_DataNum = temp;
				dsy_qspi_writepage(address, QSPI_DataNum, buffer);
			}
			else
			{
				QSPI_DataNum = size; 
				dsy_qspi_writepage(address, QSPI_DataNum, buffer);
			}
		}
		else /*!< Size > QSPI_PAGESIZE */
		{
			size -= count;
			NumOfPage =  size / flash_page_size;
			NumOfSingle = size % flash_page_size;
			QSPI_DataNum = count;
			dsy_qspi_writepage(address, QSPI_DataNum, buffer);
			address +=  count;
			buffer += count;
      
			while (NumOfPage--)
			{
				QSPI_DataNum = flash_page_size;
				dsy_qspi_writepage(address, QSPI_DataNum, buffer);
				address +=  flash_page_size;
				buffer += flash_page_size;
			}
      
			if (NumOfSingle != 0)
			{
				QSPI_DataNum = NumOfSingle;
				dsy_qspi_writepage(address, QSPI_DataNum, buffer);
			}
		}
	}
	return MEMORY_OK;	
}
int dsy_qspi_erase(uint32_t start_adr, uint32_t end_adr)
{
	uint32_t block_addr;
	uint32_t block_size = 0x10000; // 64kB blocks for now.
	// 64kB chunks for now.
	start_adr = start_adr - (start_adr % block_size);
	while (end_adr >= start_adr)
	{
		block_addr = start_adr & 0x0FFFFFFF;	
		if (dsy_qspi_erasesector(block_addr) != MEMORY_OK)
		{
			return MEMORY_ERROR;
		}
		start_adr += block_size;
	}
	return MEMORY_OK;
}

int dsy_qspi_erasesector(uint32_t addr)
{
	QSPI_CommandTypeDef s_command;
	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction = SUBSECTOR_ERASE_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.NbData            = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
	if (write_enable(&dsy_qspi_handle) != MEMORY_OK)
	{
		return MEMORY_ERROR;
	}
	if (HAL_QSPI_Command(&dsy_qspi_handle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != MEMORY_OK)
	{
		return MEMORY_ERROR;
	}
	if (autopolling_mem_ready(&dsy_qspi_handle, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != MEMORY_OK)
	{
		return MEMORY_ERROR;
	}
	return MEMORY_OK;	
}

/* Static Function Implementation */
static uint32_t reset_memory(QSPI_HandleTypeDef *hqspi)
{
	QSPI_CommandTypeDef s_command;

	/* Initialize the reset enable command */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = RESET_ENABLE_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Send the command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return MEMORY_ERROR;
	}

	/* Send the reset memory command */
	s_command.Instruction = RESET_MEMORY_CMD;
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return MEMORY_ERROR;
	}

	/* Configure automatic polling mode to wait the memory is ready */
	if (autopolling_mem_ready(hqspi, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != MEMORY_OK)
	{
		return MEMORY_ERROR;
	}
	return MEMORY_OK;
}
static uint32_t dummy_cycles_cfg(QSPI_HandleTypeDef *hqspi)
{
	QSPI_CommandTypeDef s_command;
	uint16_t reg = 0;

	/* Initialize the read volatile configuration register command */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = READ_READ_PARAM_REG_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_1_LINE;
	s_command.DummyCycles       = 0;
	s_command.NbData            = 1;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Configure the command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return MEMORY_ERROR;
	}

	/* Reception of the data */
	if (HAL_QSPI_Receive(hqspi, (uint8_t *)(&reg), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return MEMORY_ERROR;
	}

	/* Enable write operations */
	if (write_enable(hqspi) != MEMORY_OK)
	{
		return MEMORY_ERROR;
	}

	/* Update volatile configuration register (with new dummy cycles) */
	s_command.Instruction = WRITE_READ_PARAM_REG_CMD;
	MODIFY_REG(reg, 0x78, (IS25LP080D_DUMMY_CYCLES_READ_QUAD << 3));

	/* Configure the write volatile configuration register command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return MEMORY_ERROR;
	}

	/* Transmission of the data */
	if (HAL_QSPI_Transmit(hqspi, (uint8_t *)(&reg), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return MEMORY_ERROR;
	}
	
	/* Configure automatic polling mode to wait the memory is ready */
	if (autopolling_mem_ready(hqspi, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != MEMORY_OK)
	{
		return MEMORY_ERROR;
	}
	return MEMORY_OK;
}
static uint32_t write_enable(QSPI_HandleTypeDef *hqspi)
{
	QSPI_CommandTypeDef     s_command;
	QSPI_AutoPollingTypeDef s_config;

	/* Enable write operations */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = WRITE_ENABLE_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return MEMORY_ERROR;
	}

	/* Configure automatic polling mode to wait for write enabling */
//		s_config.Match           = IS25LP080D_SR_WREN | (IS25LP080D_SR_WREN << 8);
//		s_config.Mask            = IS25LP080D_SR_WREN | (IS25LP080D_SR_WREN << 8);
		s_config.MatchMode       = QSPI_MATCH_MODE_AND;
	s_config.Match = IS25LP080D_SR_WREN;
	s_config.Mask = IS25LP080D_SR_WREN;
	s_config.Interval        = 0x10;
	s_config.StatusBytesSize = 1;
	s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

	s_command.Instruction    = READ_STATUS_REG_CMD;
	s_command.DataMode       = QSPI_DATA_1_LINE;

	if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return MEMORY_ERROR;
	}
	return MEMORY_OK;
}
static uint32_t quad_enable(QSPI_HandleTypeDef *hqspi)
{
	QSPI_CommandTypeDef     s_command;
	QSPI_AutoPollingTypeDef s_config;
	uint8_t reg = 0;

	/* Enable write operations */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = WRITE_STATUS_REG_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_1_LINE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Enable write operations */
	if (write_enable(hqspi) != MEMORY_OK)
	{
		return MEMORY_ERROR;
	}

	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return MEMORY_ERROR;
	}


//	reg = 0;
//	MODIFY_REG(reg,
//		0xF0,
//		(IS25LP08D_SR_QE));
	reg = IS25LP080D_SR_QE; // Set QE bit  to 1
	/* Transmission of the data */
	if (HAL_QSPI_Transmit(hqspi, (uint8_t *)(&reg), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return MEMORY_ERROR;
	}

	/* Configure automatic polling mode to wait for write enabling */
	//	s_config.Match           = IS25LP08D_SR_WREN | (IS25LP08D_SR_WREN << 8);
	//	s_config.Mask            = IS25LP08D_SR_WREN | (IS25LP08D_SR_WREN << 8);
	//	s_config.MatchMode       = QSPI_MATCH_MODE_AND;
	//	s_config.StatusBytesSize = 2;
	s_config.Match = IS25LP080D_SR_QE;
	s_config.Mask = IS25LP080D_SR_QE;
		s_config.MatchMode       = QSPI_MATCH_MODE_AND;
		s_config.StatusBytesSize = 2;

	s_config.Interval        = 0x10;
	s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

	s_command.Instruction    = READ_STATUS_REG_CMD;
	s_command.DataMode       = QSPI_DATA_1_LINE;

	if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return MEMORY_ERROR;
	}

	/* Configure automatic polling mode to wait the memory is ready */
	if (autopolling_mem_ready(hqspi, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != MEMORY_OK)
	{
		return MEMORY_ERROR;
	}
	return MEMORY_OK;
}
static uint32_t enable_memory_mapped_mode(QSPI_HandleTypeDef *hqspi)
{
	QSPI_CommandTypeDef      s_command;
	QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;

	/* Configure the command for the read instruction */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = QUAD_OUT_FAST_READ_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
	s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_4_LINES;
	s_command.DummyCycles       = IS25LP080D_DUMMY_CYCLES_READ_QUAD;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Configure the memory mapped mode */
	s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
	s_mem_mapped_cfg.TimeOutPeriod     = 0;

	if (HAL_QSPI_MemoryMapped(hqspi, &s_command, &s_mem_mapped_cfg) != HAL_OK)
	{
		return MEMORY_ERROR;
	}
	return MEMORY_OK;
}
static uint32_t autopolling_mem_ready(QSPI_HandleTypeDef *hqspi, uint32_t timeout)
{
	QSPI_CommandTypeDef     s_command;
	QSPI_AutoPollingTypeDef s_config;

	/* Configure automatic polling mode to wait for memory ready */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = READ_STATUS_REG_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_1_LINE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	s_config.Match           = 0;
	s_config.MatchMode       = QSPI_MATCH_MODE_AND;
	s_config.Interval        = 0x10;
	s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;
	s_config.Mask            = IS25LP080D_SR_WIP;
	//s_config.Mask            = 0;
	s_config.StatusBytesSize = 1;

	if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, timeout) != HAL_OK)
	{
		return MEMORY_ERROR;
	}
	return MEMORY_OK;
}
static uint32_t enter_quad_mode(QSPI_HandleTypeDef *hqspi)
{
	QSPI_CommandTypeDef s_command;
	uint8_t reg = 0;

	/* Initialize the read volatile configuration register command */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = ENTER_QUAD_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.NbData            = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Configure the command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != MEMORY_OK)
	{
		return MEMORY_ERROR;
	}
//	/* Wait for WIP bit in SR */
//	if (QSPI_AutoPollingMemReady(hqspi, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != MEMORY_OK)
//	{
//		return MEMORY_ERROR;
//	}
	return MEMORY_OK;
}
static uint32_t exit_quad_mode(QSPI_HandleTypeDef *hqspi)
{
	QSPI_CommandTypeDef s_command;
	uint8_t reg = 0;

	/* Initialize the read volatile configuration register command */
	s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
	s_command.Instruction       = EXIT_QUAD_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.NbData            = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Configure the command */
	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != MEMORY_OK)
	{
		return MEMORY_ERROR;
	}
	/* Wait for WIP bit in SR */
	if (autopolling_mem_ready(hqspi, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != MEMORY_OK)
	{
		return MEMORY_ERROR;
	}
	return MEMORY_OK;
}

static uint8_t get_status_register(QSPI_HandleTypeDef *hqspi)
{
	QSPI_CommandTypeDef s_command;
	uint8_t reg;
	reg = 0x00;
	s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction = READ_STATUS_REG_CMD;
	s_command.AddressMode = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode = QSPI_DATA_1_LINE;
	s_command.DummyCycles = 0;
	s_command.NbData = 1;
	s_command.DdrMode = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return 0x00;
	}
	if (HAL_QSPI_Receive(hqspi, (uint8_t*)(&reg), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
	{
		return 0x00;
	}
	return reg;
}

/* HAL Overwrite Implementation */

void HAL_QSPI_MspInit(QSPI_HandleTypeDef* qspiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(qspiHandle->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspInit 0 */

  /* USER CODE END QUADSPI_MspInit 0 */
    /* QUADSPI clock enable */
    __HAL_RCC_QSPI_CLK_ENABLE();
  
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**QUADSPI GPIO Configuration    
    PG6     ------> QUADSPI_BK1_NCS
    PF7     ------> QUADSPI_BK1_IO2
    PF6     ------> QUADSPI_BK1_IO3
    PF9     ------> QUADSPI_BK1_IO1
    PF8     ------> QUADSPI_BK1_IO0
    PB2     ------> QUADSPI_CLK 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* QUADSPI interrupt Init */
    HAL_NVIC_SetPriority(QUADSPI_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(QUADSPI_IRQn);
  /* USER CODE BEGIN QUADSPI_MspInit 1 */

  /* USER CODE END QUADSPI_MspInit 1 */
  }
}

void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* qspiHandle)
{

  if(qspiHandle->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspDeInit 0 */

  /* USER CODE END QUADSPI_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_QSPI_CLK_DISABLE();
  
    /**QUADSPI GPIO Configuration    
    PG6     ------> QUADSPI_BK1_NCS
    PF7     ------> QUADSPI_BK1_IO2
    PF6     ------> QUADSPI_BK1_IO3
    PF9     ------> QUADSPI_BK1_IO1
    PF8     ------> QUADSPI_BK1_IO0
    PB2     ------> QUADSPI_CLK 
    */
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_7|GPIO_PIN_6|GPIO_PIN_9|GPIO_PIN_8);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_2);

    /* QUADSPI interrupt Deinit */
    HAL_NVIC_DisableIRQ(QUADSPI_IRQn);
  /* USER CODE BEGIN QUADSPI_MspDeInit 1 */

  /* USER CODE END QUADSPI_MspDeInit 1 */
  }
} 

void QUADSPI_IRQHandler(void)
{
  /* USER CODE BEGIN QUADSPI_IRQn 0 */

  /* USER CODE END QUADSPI_IRQn 0 */
  HAL_QSPI_IRQHandler(&dsy_qspi_handle);
  /* USER CODE BEGIN QUADSPI_IRQn 1 */

  /* USER CODE END QUADSPI_IRQn 1 */
}
