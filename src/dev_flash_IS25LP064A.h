#ifndef IS25LP064A_H
#define IS25LP064A_H /**< & */
#ifdef __cplusplus
extern "C"
{
#endif


#define IS25LP064A_FLASH_SIZE \
    0x800000 /**< 2 * 8 MBits => 1 * 1MBytes => 1MBytes*/
#define IS25LP064A_SECTOR_SIZE 0x10000   /**< 2 * 1024 sectors of 64KBytes */
#define IS25LP064A_SUBSECTOR_SIZE 0x1000 /**< 2 * 16384 subsectors of 4kBytes */
#define IS25LP064A_PAGE_SIZE 0x100       /**< 2 * 262144 pages of 256 bytes */

#define IS25LP064A_DUMMY_CYCLES_READ_QUAD 8     /**< & */
#define IS25LP064A_DUMMY_CYCLES_READ 8          /**< & */
#define IS25LP064A_DUMMY_CYCLES_READ_DTR 6      /**< & */
#define IS25LP064A_DUMMY_CYCLES_READ_QUAD_DTR 6 /**< & */


#define IS25LP064A_DIE_ERASE_MAX_TIME 460000    /**< & */
#define IS25LP064A_SECTOR_ERASE_MAX_TIME 1000   /**< & */
#define IS25LP064A_SUBSECTOR_ERASE_MAX_TIME 400 /**< & */

    /**
     * @brief  IS25LP08D Commands  
     */

    /** @addtogroup flash
    @{
    */

//problems:
    // Quad in page program (32/38) is listed as a different command
    // No address: hold, erase
    // Chip erase (C7/60)
    // Blocks/sectors are called sectors/subsectors
    // Read/Write function register (48/42) respectively
    // program/erase suspend & resume (what about 30/B0?)
    // enter/release deep power down (b9,ab)
    // set read parameters (c0) (WRITE_READ_PARAM_REG_CMD)
    // read product id (ab)
    // read device manufacturer & device id (90)
    // read unique id (4b) (READ_OTP_ARRAY)
    // no-op (00) (cancels CE)
    // security information row?
    // info row erase/program/read (64,62,68)
    // sector lock/unlock  (24,26)
    
/** Reset Operations */
#define RESET_ENABLE_CMD 0x66 
#define RESET_MEMORY_CMD 0x99 /**< & */ 

    /** Identification Operations */
#define READ_ID_CMD 0x9E 
#define READ_ID_CMD2 0x9F                      /**< & */ 
#define MULTIPLE_IO_READ_ID_CMD 0xAF           /**< & */ 
#define READ_SERIAL_FLASH_DISCO_PARAM_CMD 0x5A /**< & */ 

    /** Read Operations */
#define READ_CMD 0x03  

#define FAST_READ_CMD 0x0B             /**< & */ 
#define FAST_READ_DTR_CMD 0x0D         /**< & */ 

#define DUAL_OUT_FAST_READ_CMD 0x3B             /**< & */ 

#define DUAL_INOUT_FAST_READ_CMD 0xBB             /**< & */ 
#define DUAL_INOUT_FAST_READ_DTR_CMD 0xBD         /**< & */ 

#define QUAD_OUT_FAST_READ_CMD 0x6B             /**< & */ 

#define QUAD_INOUT_FAST_READ_CMD 0xEB             /**< & */ 
#define QUAD_INOUT_FAST_READ_DTR_CMD 0xED         /**< & */ 

    /** Write Operations */
#define WRITE_ENABLE_CMD 0x06 
#define WRITE_DISABLE_CMD 0x04 /**< & */ 

    /** Register Operations */
#define READ_STATUS_REG_CMD 0x05 
#define WRITE_STATUS_REG_CMD 0x01 /**< & */ 

#define READ_READ_PARAM_REG_CMD 0x61  /**< & */ ???
#define WRITE_READ_PARAM_REG_CMD 0xC0 /**< & */ ???

    /** Program Operations */
#define PAGE_PROG_CMD 0x02 

#define QUAD_IN_FAST_PROG_CMD 0x32             /**< & */ 
#define EXT_QUAD_IN_FAST_PROG_CMD 0x38         /**< & */ 

    /** Erase Operations */
#define SUBSECTOR_ERASE_CMD 0xd7 
#define SUBSECTOR_ERASE_QPI_CMD 0x20         /**< & */ 

#define SECTOR_ERASE_CMD 0xD8             /**< & */  

#define PROG_ERASE_RESUME_CMD 0x7A  /**< & */ //0x30
#define PROG_ERASE_SUSPEND_CMD 0x75 /**< & */ //0xB0


    /** Quad Operations */
#define ENTER_QUAD_CMD 0x35 
#define EXIT_QUAD_CMD 0xF5 /**< & */  

    /** 
                      * @brief  IS25LP08D Registers  
                      */
    /* Status Register */
#define IS25LP064A_SR_WIP ((uint8_t)0x01)  /*!< Write in progress */
#define IS25LP064A_SR_WREN ((uint8_t)0x02) /*!< Write enable latch */
//#define IS25LP064A_SR_BLOCKPR                  ((uint8_t)0x5C)    /*!< Block protected against program and erase operations */
//#define IS25LP064A_SR_PRBOTTOM                 ((uint8_t)0x20)    /*!< Protected memory area defined by BLOCKPR starts from top or bottom */
#define IS25LP064A_SR_SRWREN \
    ((uint8_t)0x80) /*!< Status register write enable/disable */
#define IS25LP064A_SR_QE ((uint8_t)0x40) /**< & */

    /* Non volatile Configuration Register */
#define IS25LP064A_NVCR_NBADDR \
    ((uint16_t)0x0001) /*!< 3-bytes or 4-bytes addressing */
#define IS25LP064A_NVCR_SEGMENT \
    ((uint16_t)0x0002) /*!< Upper or lower 128Mb segment selected by default */
#define IS25LP064A_NVCR_DUAL ((uint16_t)0x0004) /*!< Dual I/O protocol */
#define IS25LP064A_NVCR_QUAB ((uint16_t)0x0008) /*!< Quad I/O protocol */
#define IS25LP064A_NVCR_RH ((uint16_t)0x0010)   /*!< Reset/hold */
#define IS25LP064A_NVCR_DTRP \
    ((uint16_t)0x0020) /*!< Double transfer rate protocol */
#define IS25LP064A_NVCR_ODS ((uint16_t)0x01C0) /*!< Output driver strength */
#define IS25LP064A_NVCR_XIP \
    ((uint16_t)0x0E00) /*!< XIP mode at power-on reset */
#define IS25LP064A_NVCR_NB_DUMMY \
    ((uint16_t)0xF000) /*!< Number of dummy clock cycles */

    /* Volatile Configuration Register */
#define IS25LP064A_VCR_WRAP ((uint8_t)0x03) /*!< Wrap */
#define IS25LP064A_VCR_XIP ((uint8_t)0x08)  /*!< XIP */
#define IS25LP064A_VCR_NB_DUMMY \
    ((uint8_t)0xF0) /*!< Number of dummy clock cycles */

    /* Extended Address Register */
#define IS25LP064A_EAR_HIGHEST_SE \
    ((uint8_t)0x03) /*!< Select the Highest 128Mb segment */
#define IS25LP064A_EAR_THIRD_SEG \
    ((uint8_t)0x02) /*!< Select the Third 128Mb segment */
#define IS25LP064A_EAR_SECOND_SEG \
    ((uint8_t)0x01) /*!< Select the Second 128Mb segment */
#define IS25LP064A_EAR_LOWEST_SEG \
    ((uint8_t)0x00) /*!< Select the Lowest 128Mb segment (default) */

    /* Enhanced Volatile Configuration Register */
#define IS25LP064A_EVCR_ODS ((uint8_t)0x07) /*!< Output driver strength */
#define IS25LP064A_EVCR_RH ((uint8_t)0x10)  /*!< Reset/hold */
#define IS25LP064A_EVCR_DTRP \
    ((uint8_t)0x20) /*!< Double transfer rate protocol */
#define IS25LP064A_EVCR_DUAL ((uint8_t)0x40) /*!< Dual I/O protocol */
#define IS25LP064A_EVCR_QUAD ((uint8_t)0x80) /*!< Quad I/O protocol */

    /* Flag Status Register */
#define IS25LP064A_FSR_NBADDR \
    ((uint8_t)0x01) /*!< 3-bytes or 4-bytes addressing */
#define IS25LP064A_FSR_PRERR ((uint8_t)0x02) /*!< Protection error */
#define IS25LP064A_FSR_PGSUS ((uint8_t)0x04) /*!< Program operation suspended */
#define IS25LP064A_FSR_PGERR ((uint8_t)0x10) /*!< Program error */
#define IS25LP064A_FSR_ERERR ((uint8_t)0x20) /*!< Erase error */
#define IS25LP064A_FSR_ERSUS ((uint8_t)0x40) /*!< Erase operation suspended */
#define IS25LP064A_FSR_READY \
    ((uint8_t)0x80) /*!< Ready or command in progress */


#ifdef __cplusplus
}
#endif
#endif
/** @} */
