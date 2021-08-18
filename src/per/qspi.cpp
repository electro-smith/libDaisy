#include "per/qspi.h"
#include "stm32h7xx_hal.h"
#include "dev/flash_IS25LP080D.h"
#include "dev/flash_IS25LP064A.h"
extern "C"
{
#include "util/hal_map.h"
}

// TODO: Add handling for alternate device types,
//        This will be a thing much sooner than anticipated
//        due to upgrading the RAM size for the new 4MB chip.
// TODO: AutopollingMemReady only works for 1-Line, not 4-Line

#define SET_MODE(mode)                              \
    if(mode_ != mode)                               \
    {                                               \
        mode_ = mode;                               \
        if(Init(config_) != QSPIHandle::Result::OK) \
        {                                           \
            mode_ = Mode::MEMORY_MAPPED;            \
            Init(config_);                          \
            return QSPIHandle::Result::ERR;         \
        }                                           \
    }


namespace daisy
{
/** Private implementation for QSPIHandle */
class QSPIHandle::Impl
{
  public:
    /** 
        Modes of operation.
        Memory Mapped mode: QSPI configured so that the QSPI can be
        read from starting address 0x90000000. Writing is not
        possible in this mode. \n 
        Indirect Polling mode: Device driver enabled. 
        */
    enum Mode
    {
        MEMORY_MAPPED,    /**< & */
        INDIRECT_POLLING, /**< & */
        MODE_LAST,
    };

    QSPIHandle::Result Init(const QSPIHandle::Config& config);

    const QSPIHandle::Config& GetConfig() const { return config_; }

    QSPIHandle::Result Deinit();

    QSPIHandle::Result WritePage(uint32_t address,
                                 uint32_t size,
                                 uint8_t* buffer,
                                 bool     reset_mode = true);

    QSPIHandle::Result Write(uint32_t address, uint32_t size, uint8_t* buffer);

    QSPIHandle::Result Erase(uint32_t start_addr, uint32_t end_addr);

    QSPIHandle::Result EraseSector(uint32_t address);

    uint32_t GetPin(size_t pin);

    GPIO_TypeDef* GetPort(size_t pin);

    Mode GetMode() { return mode_; }

    QSPI_HandleTypeDef* GetHalHandle();

    size_t GetNumPins() { return pin_count_; }

  private:
    QSPIHandle::Result ResetMemory();

    QSPIHandle::Result DummyCyclesConfig(QSPIHandle::Config::Device device);

    QSPIHandle::Result WriteEnable();

    QSPIHandle::Result QuadEnable();

    QSPIHandle::Result EnableMemoryMappedMode();

    QSPIHandle::Result AutopollingMemReady(uint32_t timeout);

    // These functions are defined, but we haven't added the ability to switch to quad mode. So they're currently unused.
    QSPIHandle::Result EnterQuadMode() __attribute__((unused));
    QSPIHandle::Result ExitQuadMode() __attribute__((unused));
    uint8_t            GetStatusRegister() __attribute__((unused));

    QSPIHandle::Config config_;
    QSPI_HandleTypeDef halqspi_;
    Mode               mode_;

    static constexpr size_t pin_count_
        = sizeof(QSPIHandle::Config::pin_config) / sizeof(dsy_gpio_pin);
    // Data structure for easy hal initialization
    dsy_gpio_pin* pin_config_arr_[pin_count_] = {&config_.pin_config.io0,
                                                 &config_.pin_config.io1,
                                                 &config_.pin_config.io2,
                                                 &config_.pin_config.io3,
                                                 &config_.pin_config.clk,
                                                 &config_.pin_config.ncs};
};


// ================================================================
// Global reference to the handle for interfacing with hal
// ================================================================

static QSPIHandle::Impl qspi_impl;


QSPIHandle::Result QSPIHandle::Impl::Init(const QSPIHandle::Config& config)
{
    // Set Handle Settings     o

    config_ = config;
    QSPIHandle::Config::Device device;
    device = config_.device;

    if(HAL_QSPI_DeInit(&halqspi_) != HAL_OK)
    {
        return QSPIHandle::Result::ERR;
    }
    //HAL_QSPI_MspInit(&dsy_qspi_handle);     // I think this gets called a in HAL_QSPI_Init();
    // Set Initialization values for the QSPI Peripheral
    uint32_t flash_size;
    switch(device)
    {
        case QSPIHandle::Config::Device::IS25LP080D:
            flash_size = IS25LP080D_FLASH_SIZE;
            break;
        case QSPIHandle::Config::Device::IS25LP064A:
            flash_size = IS25LP064A_FLASH_SIZE;
            break;
        default: flash_size = IS25LP080D_FLASH_SIZE; break;
    }
    halqspi_.Instance = QUADSPI;
    //dsy_qspi_handle.Init.ClockPrescaler = 7;
    //dsy_qspi_handle.Init.ClockPrescaler = 7;
    //dsy_qspi_handle.Init.ClockPrescaler = 2; // Conservative setting for now. Signal gets very weak faster than this.
    halqspi_.Init.ClockPrescaler     = 1;
    halqspi_.Init.FifoThreshold      = 1;
    halqspi_.Init.SampleShifting     = QSPI_SAMPLE_SHIFTING_NONE;
    halqspi_.Init.FlashSize          = POSITION_VAL(flash_size) - 1;
    halqspi_.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;
    halqspi_.Init.FlashID            = QSPI_FLASH_ID_1;
    halqspi_.Init.DualFlash          = QSPI_DUALFLASH_DISABLE;

    if(HAL_QSPI_Init(&halqspi_) != HAL_OK)
    {
        return QSPIHandle::Result::ERR;
    }
    if(ResetMemory() != QSPIHandle::Result::OK)
    {
        return QSPIHandle::Result::ERR;
    }
    //    uint8_t fifothresh = HAL_QSPI_GetFifoThreshold(&dsy_qspi_handle.hqspi);
    //    uint8_t reg = 0;
    //    reg = get_status_register(&dsy_qspi_handle);
    if(DummyCyclesConfig(device) != QSPIHandle::Result::OK)
    {
        return QSPIHandle::Result::ERR;
    }
    // Once writing test with 1 Line is confirmed lets move this out, and update writing to use 4-line.
    if(QuadEnable() != QSPIHandle::Result::OK)
    {
        return QSPIHandle::Result::ERR;
    }

    if(mode_ == Mode::MEMORY_MAPPED)
    {
        if(EnableMemoryMappedMode() != QSPIHandle::Result::OK)
        {
            return QSPIHandle::Result::ERR;
        }
    }

    return QSPIHandle::Result::OK;
}


QSPIHandle::Result QSPIHandle::Impl::Deinit()
{
    halqspi_.Instance = QUADSPI;
    if(HAL_QSPI_DeInit(&halqspi_) != HAL_OK)
    {
        return QSPIHandle::Result::ERR;
    }
    HAL_QSPI_MspDeInit(&halqspi_);
    return QSPIHandle::Result::OK;
}


QSPIHandle::Result QSPIHandle::Impl::WritePage(uint32_t address,
                                               uint32_t size,
                                               uint8_t* buffer,
                                               bool     reset_mode)
{
    SET_MODE(Mode::INDIRECT_POLLING);

    QSPI_CommandTypeDef s_command;
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = PAGE_PROG_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_1_LINE;
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.DummyCycles       = 0;
    s_command.NbData            = size <= 256 ? size : 256;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    s_command.Address           = address;
    if(WriteEnable() != QSPIHandle::Result::OK)
    {
        goto write_error;
    }
    if(HAL_QSPI_Command(&halqspi_, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != HAL_OK)
    {
        goto write_error;
    }
    if(HAL_QSPI_Transmit(
           &halqspi_, (uint8_t*)buffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != HAL_OK)
    {
        goto write_error;
    }
    if(AutopollingMemReady(HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != QSPIHandle::Result::OK)
    {
        goto write_error;
    }

    if(reset_mode)
        SET_MODE(Mode::MEMORY_MAPPED);
    return QSPIHandle::Result::OK;

// I'm sorry, but it does minimize code size here
write_error:
    if(reset_mode)
        SET_MODE(Mode::MEMORY_MAPPED);
    return QSPIHandle::Result::ERR;
}


QSPIHandle::Result
QSPIHandle::Impl::Write(uint32_t address, uint32_t size, uint8_t* buffer)
{
    uint32_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
    uint32_t QSPI_DataNum    = 0;
    uint32_t flash_page_size = IS25LP080D_PAGE_SIZE;
    address                  = address & 0x0FFFFFFF;
    Addr                     = address % flash_page_size;
    count                    = flash_page_size - Addr;
    NumOfPage                = size / flash_page_size;
    NumOfSingle              = size % flash_page_size;

    if(Addr == 0) /*!< Address is QSPI_PAGESIZE aligned  */
    {
        if(NumOfPage == 0) /*!< NumByteToWrite < QSPI_PAGESIZE */
        {
            QSPI_DataNum = size;
            WritePage(address, QSPI_DataNum, buffer, false);
        }
        else /*!< Size > QSPI_PAGESIZE */
        {
            while(NumOfPage--)
            {
                QSPI_DataNum = flash_page_size;
                WritePage(address, QSPI_DataNum, buffer, false);
                address += flash_page_size;
                buffer += flash_page_size;
            }

            QSPI_DataNum = NumOfSingle;
            if(QSPI_DataNum > 0)
                WritePage(address, QSPI_DataNum, buffer, false);
        }
    }
    else /*!< Address is not QSPI_PAGESIZE aligned  */
    {
        if(NumOfPage == 0) /*!< Size < QSPI_PAGESIZE */
        {
            if(NumOfSingle > count) /*!< (Size + Address) > QSPI_PAGESIZE */
            {
                temp         = NumOfSingle - count;
                QSPI_DataNum = count;
                WritePage(address, QSPI_DataNum, buffer, false);
                address += count;
                buffer += count;
                QSPI_DataNum = temp;
                WritePage(address, QSPI_DataNum, buffer, false);
            }
            else
            {
                QSPI_DataNum = size;
                WritePage(address, QSPI_DataNum, buffer, false);
            }
        }
        else /*!< Size > QSPI_PAGESIZE */
        {
            size -= count;
            NumOfPage    = size / flash_page_size;
            NumOfSingle  = size % flash_page_size;
            QSPI_DataNum = count;
            WritePage(address, QSPI_DataNum, buffer, false);
            address += count;
            buffer += count;

            while(NumOfPage--)
            {
                QSPI_DataNum = flash_page_size;
                WritePage(address, QSPI_DataNum, buffer, false);
                address += flash_page_size;
                buffer += flash_page_size;
            }

            if(NumOfSingle != 0)
            {
                QSPI_DataNum = NumOfSingle;
                WritePage(address, QSPI_DataNum, buffer, false);
            }
        }
    }

    SET_MODE(Mode::MEMORY_MAPPED);
    return QSPIHandle::Result::OK;
}


QSPIHandle::Result QSPIHandle::Impl::Erase(uint32_t start_addr,
                                           uint32_t end_addr)
{
    uint32_t block_addr;
    uint32_t block_size = IS25LP080D_SECTOR_SIZE; // 4kB blocks for now.
    // 64kB chunks for now.
    start_addr = start_addr - (start_addr % block_size);
    while(end_addr >= start_addr)
    {
        block_addr = start_addr & 0x0FFFFFFF;
        if(EraseSector(block_addr) != QSPIHandle::Result::OK)
        {
            return QSPIHandle::Result::ERR;
        }
        start_addr += block_size;
    }
    return QSPIHandle::Result::OK;
}


QSPIHandle::Result QSPIHandle::Impl::EraseSector(uint32_t address)
{
    uint8_t             use_qpi = 0;
    QSPI_CommandTypeDef s_command;
    if(use_qpi)
    {
        s_command.InstructionMode = QSPI_INSTRUCTION_4_LINES;
        s_command.Instruction     = SECTOR_ERASE_QPI_CMD;
        s_command.AddressMode     = QSPI_ADDRESS_4_LINES;
    }
    else
    {
        s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
        s_command.Instruction     = SECTOR_ERASE_CMD;
        s_command.AddressMode     = QSPI_ADDRESS_1_LINE;
    }
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_NONE;
    s_command.DummyCycles       = 0;
    s_command.NbData            = 0;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    s_command.Address           = address;

    // Erasing takes a long time anyway, so not much point trying to
    // minimize reinitializations
    SET_MODE(Mode::INDIRECT_POLLING);

    if(WriteEnable() != QSPIHandle::Result::OK)
    {
        goto erase_error;
    }
    if(HAL_QSPI_Command(&halqspi_, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != HAL_OK)
    {
        goto erase_error;
    }
    if(AutopollingMemReady(HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != QSPIHandle::Result::OK)
    {
        goto erase_error;
    }

    SET_MODE(Mode::MEMORY_MAPPED);
    return QSPIHandle::Result::OK;

// I'm sorry, but it does minimize code size here
erase_error:
    SET_MODE(Mode::MEMORY_MAPPED);
    return QSPIHandle::Result::ERR;
}


QSPIHandle::Result QSPIHandle::Impl::ResetMemory()
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
    if(HAL_QSPI_Command(&halqspi_, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != HAL_OK)
    {
        return QSPIHandle::Result::ERR;
    }

    /* Send the reset memory command */
    s_command.Instruction = RESET_MEMORY_CMD;
    if(HAL_QSPI_Command(&halqspi_, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != HAL_OK)
    {
        return QSPIHandle::Result::ERR;
    }

    /* Configure automatic polling mode to wait the memory is ready */
    if(AutopollingMemReady(HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != QSPIHandle::Result::OK)
    {
        return QSPIHandle::Result::ERR;
    }
    return QSPIHandle::Result::OK;
}


QSPIHandle::Result
QSPIHandle::Impl::DummyCyclesConfig(QSPIHandle::Config::Device device)
{
    QSPI_CommandTypeDef s_command;
    uint16_t            reg     = 0;
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.DummyCycles       = 0;
    s_command.NbData            = 1;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    if(device == QSPIHandle::Config::Device::IS25LP080D)
    {
        /* Initialize the read volatile configuration register command */
        s_command.Instruction = READ_READ_PARAM_REG_CMD;

        /* Configure the command */
        if(HAL_QSPI_Command(
               &halqspi_, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
           != HAL_OK)
        {
            return QSPIHandle::Result::ERR;
        }

        /* Reception of the data */
        if(HAL_QSPI_Receive(
               &halqspi_, (uint8_t*)(&reg), HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
           != HAL_OK)
        {
            return QSPIHandle::Result::ERR;
        }
        MODIFY_REG(reg, 0x78, (IS25LP080D_DUMMY_CYCLES_READ_QUAD << 3));
        /* Enable write operations */
        if(WriteEnable() != QSPIHandle::Result::OK)
        {
            return QSPIHandle::Result::ERR;
        }
    }
    else
    {
        // Only volatile Read Params on 16MB chip.
        // Explicitly set:
        // Burst Length: 8 bytes (0, 0)
        // Wrap Enable: 0
        // Dummy Cycles: (Config 3, bits 1 0)
        // Drive Strength (50%, bits 1 1 1)
        // Byte to write: 0b11110000 (0xF0)
        // TODO: Probably expand Burst to maximum if that works out.

        reg = 0xF0;
    }


    /* Update volatile configuration register (with new dummy cycles) */
    s_command.Instruction = WRITE_READ_PARAM_REG_CMD;

    /* Configure the write volatile configuration register command */
    if(HAL_QSPI_Command(&halqspi_, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != HAL_OK)
    {
        return QSPIHandle::Result::ERR;
    }

    /* Transmission of the data */
    if(HAL_QSPI_Transmit(
           &halqspi_, (uint8_t*)(&reg), HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != HAL_OK)
    {
        return QSPIHandle::Result::ERR;
    }

    /* Configure automatic polling mode to wait the memory is ready */
    if(AutopollingMemReady(HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != QSPIHandle::Result::OK)
    {
        return QSPIHandle::Result::ERR;
    }
    return QSPIHandle::Result::OK;
}


QSPIHandle::Result QSPIHandle::Impl::WriteEnable()
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

    if(HAL_QSPI_Command(&halqspi_, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != HAL_OK)
    {
        return QSPIHandle::Result::ERR;
    }

    /* Configure automatic polling mode to wait for write enabling */
    //        s_config.Match           = IS25LP080D_SR_WREN | (IS25LP080D_SR_WREN << 8);
    //        s_config.Mask            = IS25LP080D_SR_WREN | (IS25LP080D_SR_WREN << 8);
    s_config.MatchMode       = QSPI_MATCH_MODE_AND;
    s_config.Match           = IS25LP080D_SR_WREN;
    s_config.Mask            = IS25LP080D_SR_WREN;
    s_config.Interval        = 0x10;
    s_config.StatusBytesSize = 1;
    s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

    s_command.Instruction = READ_STATUS_REG_CMD;
    s_command.DataMode    = QSPI_DATA_1_LINE;

    if(HAL_QSPI_AutoPolling(
           &halqspi_, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != HAL_OK)
    {
        return QSPIHandle::Result::ERR;
    }
    return QSPIHandle::Result::OK;
}


QSPIHandle::Result QSPIHandle::Impl::QuadEnable()
{
    QSPI_CommandTypeDef     s_command;
    QSPI_AutoPollingTypeDef s_config;
    uint8_t                 reg = 0;

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
    if(WriteEnable() != QSPIHandle::Result::OK)
    {
        return QSPIHandle::Result::ERR;
    }

    if(HAL_QSPI_Command(&halqspi_, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != HAL_OK)
    {
        return QSPIHandle::Result::ERR;
    }


    //    reg = 0;
    //    MODIFY_REG(reg,
    //        0xF0,
    //        (IS25LP08D_SR_QE));
    reg = IS25LP080D_SR_QE; // Set QE bit  to 1
    /* Transmission of the data */
    if(HAL_QSPI_Transmit(
           &halqspi_, (uint8_t*)(&reg), HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != HAL_OK)
    {
        return QSPIHandle::Result::ERR;
    }

    /* Configure automatic polling mode to wait for write enabling */
    //    s_config.Match           = IS25LP08D_SR_WREN | (IS25LP08D_SR_WREN << 8);
    //    s_config.Mask            = IS25LP08D_SR_WREN | (IS25LP08D_SR_WREN << 8);
    //    s_config.MatchMode       = QSPI_MATCH_MODE_AND;
    //    s_config.StatusBytesSize = 2;
    s_config.Match           = IS25LP080D_SR_QE;
    s_config.Mask            = IS25LP080D_SR_QE;
    s_config.MatchMode       = QSPI_MATCH_MODE_AND;
    s_config.StatusBytesSize = 1;

    s_config.Interval      = 0x10;
    s_config.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;

    s_command.Instruction = READ_STATUS_REG_CMD;
    s_command.DataMode    = QSPI_DATA_1_LINE;

    if(HAL_QSPI_AutoPolling(
           &halqspi_, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != HAL_OK)
    {
        return QSPIHandle::Result::ERR;
    }

    /* Configure automatic polling mode to wait the memory is ready */
    if(AutopollingMemReady(HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != QSPIHandle::Result::OK)
    {
        return QSPIHandle::Result::ERR;
    }
    return QSPIHandle::Result::OK;
}


QSPIHandle::Result QSPIHandle::Impl::EnableMemoryMappedMode()
{
    QSPI_CommandTypeDef      s_command;
    QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;

    /* Configure the command for the read instruction */
    s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction     = QUAD_INOUT_FAST_READ_CMD;
    s_command.AddressMode     = QSPI_ADDRESS_4_LINES;
    s_command.AddressSize     = QSPI_ADDRESS_24_BITS;
    //    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    //s_command.DummyCycles       = IS25LP080D_DUMMY_CYCLES_READ_QUAD;
    s_command.AlternateByteMode  = QSPI_ALTERNATE_BYTES_4_LINES;
    s_command.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    s_command.AlternateBytes     = 0x000000A0;
    s_command.DummyCycles        = 6;
    s_command.DdrMode            = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    //s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
    s_command.SIOOMode = QSPI_SIOO_INST_ONLY_FIRST_CMD;
    s_command.DataMode = QSPI_DATA_4_LINES;

    /* Configure the memory mapped mode */
    s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
    s_mem_mapped_cfg.TimeOutPeriod     = 0;

    if(HAL_QSPI_MemoryMapped(&halqspi_, &s_command, &s_mem_mapped_cfg)
       != HAL_OK)
    {
        return QSPIHandle::Result::ERR;
    }
    return QSPIHandle::Result::OK;
}


QSPIHandle::Result QSPIHandle::Impl::AutopollingMemReady(uint32_t timeout)
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

    s_config.Match         = 0;
    s_config.MatchMode     = QSPI_MATCH_MODE_AND;
    s_config.Interval      = 0x10;
    s_config.AutomaticStop = QSPI_AUTOMATIC_STOP_ENABLE;
    s_config.Mask          = IS25LP080D_SR_WIP;
    //s_config.Mask            = 0;
    s_config.StatusBytesSize = 1;

    if(HAL_QSPI_AutoPolling(&halqspi_, &s_command, &s_config, timeout)
       != HAL_OK)
    {
        return QSPIHandle::Result::ERR;
    }
    return QSPIHandle::Result::OK;
}


QSPIHandle::Result QSPIHandle::Impl::EnterQuadMode()
{
    QSPI_CommandTypeDef s_command;

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
    if(HAL_QSPI_Command(&halqspi_, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != HAL_OK)
    {
        return QSPIHandle::Result::ERR;
    }
    //    /* Wait for WIP bit in SR */
    //    if (QSPI_AutoPollingMemReady(&halqspi_, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != QSPIHandle::Result::OK)
    //    {
    //        return QSPIHandle::Result::ERR;
    //    }
    return QSPIHandle::Result::OK;
}


QSPIHandle::Result QSPIHandle::Impl::ExitQuadMode()
{
    QSPI_CommandTypeDef s_command;

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
    if(HAL_QSPI_Command(&halqspi_, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != HAL_OK)
    {
        return QSPIHandle::Result::ERR;
    }
    /* Wait for WIP bit in SR */
    if(AutopollingMemReady(HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != QSPIHandle::Result::OK)
    {
        return QSPIHandle::Result::ERR;
    }
    return QSPIHandle::Result::OK;
}


uint8_t QSPIHandle::Impl::GetStatusRegister()
{
    QSPI_CommandTypeDef s_command;
    uint8_t             reg;
    reg                         = 0x00;
    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    s_command.Instruction       = READ_STATUS_REG_CMD;
    s_command.AddressMode       = QSPI_ADDRESS_NONE;
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    s_command.DataMode          = QSPI_DATA_1_LINE;
    s_command.DummyCycles       = 0;
    s_command.NbData            = 1;
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if(HAL_QSPI_Command(&halqspi_, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != HAL_OK)
    {
        return 0x00;
    }
    if(HAL_QSPI_Receive(
           &halqspi_, (uint8_t*)(&reg), HAL_QPSI_TIMEOUT_DEFAULT_VALUE)
       != HAL_OK)
    {
        return 0x00;
    }
    return reg;
}


uint32_t QSPIHandle::Impl::GetPin(size_t pin)
{
    dsy_gpio_pin* p = pin_config_arr_[pin];
    return dsy_hal_map_get_pin(p);
}


GPIO_TypeDef* QSPIHandle::Impl::GetPort(size_t pin)
{
    dsy_gpio_pin* p = pin_config_arr_[pin];
    return dsy_hal_map_get_port(p);
}


QSPI_HandleTypeDef* QSPIHandle::Impl::GetHalHandle()
{
    return &halqspi_;
}


// ======================================================================
// QSPIHandle > QSPIHandle::Impl
// ======================================================================

QSPIHandle::Result QSPIHandle::Init(const QSPIHandle::Config& config)
{
    pimpl_ = &qspi_impl;
    return pimpl_->Init(config);
}

const QSPIHandle::Config& QSPIHandle::GetConfig() const
{
    return pimpl_->GetConfig();
}

QSPIHandle::Result QSPIHandle::Deinit()
{
    return pimpl_->Deinit();
}

QSPIHandle::Result
QSPIHandle::WritePage(uint32_t address, uint32_t size, uint8_t* buffer)
{
    return pimpl_->WritePage(address, size, buffer);
}

QSPIHandle::Result
QSPIHandle::Write(uint32_t address, uint32_t size, uint8_t* buffer)
{
    return pimpl_->Write(address, size, buffer);
}

QSPIHandle::Result QSPIHandle::Erase(uint32_t start_addr, uint32_t end_addr)
{
    return pimpl_->Erase(start_addr, end_addr);
}

QSPIHandle::Result QSPIHandle::EraseSector(uint32_t address)
{
    return pimpl_->EraseSector(address);
}

// ======================================================================
// HAL service functions
// ======================================================================

extern "C" void HAL_QSPI_MspInit(QSPI_HandleTypeDef* qspiHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(qspiHandle->Instance == QUADSPI)
    {
        /* QUADSPI clock enable */
        __HAL_RCC_QSPI_CLK_ENABLE();

        __HAL_RCC_GPIOG_CLK_ENABLE();
        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_GPIOE_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        // Seems the same for all pin outs so far.
        uint8_t       af_config[qspi_impl.GetNumPins()] = {GPIO_AF10_QUADSPI,
                                                     GPIO_AF10_QUADSPI,
                                                     GPIO_AF9_QUADSPI,
                                                     GPIO_AF9_QUADSPI,
                                                     GPIO_AF9_QUADSPI,
                                                     GPIO_AF10_QUADSPI};
        GPIO_TypeDef* port;
        for(uint8_t i = 0; i < qspi_impl.GetNumPins(); i++)
        {
            //            port                = (GPIO_TypeDef*)gpio_hal_port_map[qspi_handle.dsy_hqspi->pin_config[i].port];
            //            GPIO_InitStruct.Pin = gpio_hal_pin_map[qspi_handle.dsy_hqspi->pin_config[i].pin];
            port                      = qspi_impl.GetPort(i);
            GPIO_InitStruct.Pin       = qspi_impl.GetPin(i);
            GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull      = GPIO_NOPULL;
            GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = af_config[i];
            HAL_GPIO_Init(port, &GPIO_InitStruct);
        }
        /* QUADSPI interrupt Init */
        HAL_NVIC_SetPriority(QUADSPI_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(QUADSPI_IRQn);
    }
}

extern "C" void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* qspiHandle)
{
    if(qspiHandle->Instance == QUADSPI)
    {
        /* Peripheral clock disable */
        __HAL_RCC_QSPI_CLK_DISABLE();
        GPIO_TypeDef* port;
        uint16_t      pin;
        for(uint8_t i = 0; i < qspi_impl.GetNumPins(); i++)
        {
            //            port = (GPIO_TypeDef *)
            //                gpio_hal_port_map[qspi_handle.dsy_hqspi->pin_config[i].port];
            //            pin = gpio_hal_pin_map[qspi_handle.dsy_hqspi->pin_config[i].pin];
            port = qspi_impl.GetPort(i);
            pin  = qspi_impl.GetPin(i);
            HAL_GPIO_DeInit(port, pin);
        }
        /* QUADSPI interrupt Deinit */
        HAL_NVIC_DisableIRQ(QUADSPI_IRQn);
        /* USER CODE BEGIN QUADSPI_MspDeInit 1 */

        /* USER CODE END QUADSPI_MspDeInit 1 */
    }
}

extern "C" void QUADSPI_IRQHandler(void)
{
    HAL_QSPI_IRQHandler(qspi_impl.GetHalHandle());
}

} // namespace daisy

/* HAL Overwrite Implementation */

/**QUADSPI GPIO Configuration    
    On Daisy Rev3:
    PG6     ------> QUADSPI_BK1_NCS
    PF8     ------> QUADSPI_BK1_IO0
    PF9     ------> QUADSPI_BK1_IO1
    PF7     ------> QUADSPI_BK1_IO2
    PF6     ------> QUADSPI_BK1_IO3
    PB2     ------> QUADSPI_CLK 
    On Daisy Seed:
    PG6     ------> QUADSPI_BK1_NCS
    PF8     ------> QUADSPI_BK1_IO0
    PF9     ------> QUADSPI_BK1_IO1
    PF7     ------> QUADSPI_BK1_IO2
    PF6     ------> QUADSPI_BK1_IO3
    PF10    ------> QUADSPI_CLK 
    On Audio BB:
    PG6     ------> QUADSPI_BK1_NCS
    PF8     ------> QUADSPI_BK1_IO0
    PF9     ------> QUADSPI_BK1_IO1
    PE2     ------> QUADSPI_BK1_IO2
    PF6     ------> QUADSPI_BK1_IO3
    PF10    ------> QUADSPI_CLK 
    */
//enum
//{
//    DSY_QSPI_AF_PINS_NCS,
//    DSY_QSPI_AF_PINS_IO0,
//    DSY_QSPI_AF_PINS_IO1,
//    DSY_QSPI_AF_PINS_IO2,
//    DSY_QSPI_AF_PINS_IO3,
//    DSY_QSPI_AF_PINS_CLK,
//    DSY_QSPI_AF_PINS_LAST
//};
//static GPIO_TypeDef *gpio_config_ports[DSY_SYS_BOARD_LAST][DSY_QSPI_AF_PINS_LAST] = {
//    // NCS, IO0,   IO1,   IO2,   IO3,   CLK
//    {GPIOG, GPIOF, GPIOF, GPIOF, GPIOF, GPIOB}, // DAISY
//    {GPIOG, GPIOF, GPIOF, GPIOF, GPIOF, GPIOF}, // DAISY SEED
//    {GPIOG, GPIOF, GPIOF, GPIOE, GPIOF, GPIOF}, // AUDIO BB
//};
//static uint16_t gpio_config_pins[DSY_SYS_BOARD_LAST][DSY_QSPI_AF_PINS_LAST] = {
//    // NCS,         IO0,         IO1,         IO2,         IO3,         CLK
//    {GPIO_PIN_6, GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_7, GPIO_PIN_6, GPIO_PIN_2}, // DAISY
//    {GPIO_PIN_6, GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_7, GPIO_PIN_6, GPIO_PIN_10}, // DAISY SEED
//    {GPIO_PIN_6, GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_2, GPIO_PIN_6, GPIO_PIN_10}, // AUDIO BB
//};
//
//static uint8_t gpio_config_af[DSY_SYS_BOARD_LAST][DSY_QSPI_AF_PINS_LAST] = {
//    // NCS,                IO0,                IO1,                IO2,                IO3,           CLK
//    {GPIO_AF10_QUADSPI, GPIO_AF10_QUADSPI, GPIO_AF10_QUADSPI, GPIO_AF9_QUADSPI, GPIO_AF9_QUADSPI, GPIO_AF9_QUADSPI}, // DAISY
//    {GPIO_AF10_QUADSPI, GPIO_AF10_QUADSPI, GPIO_AF10_QUADSPI, GPIO_AF9_QUADSPI, GPIO_AF9_QUADSPI, GPIO_AF9_QUADSPI}, // DAISY SEED
//    {GPIO_AF10_QUADSPI, GPIO_AF10_QUADSPI, GPIO_AF10_QUADSPI, GPIO_AF9_QUADSPI, GPIO_AF9_QUADSPI, GPIO_AF9_QUADSPI}, // AUDIO BB
//};
//