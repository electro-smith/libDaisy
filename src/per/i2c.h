#pragma once
#include "daisy_core.h"

namespace daisy
{
/** A handle for interacting with an I2C peripheral. This is a dumb
 *  gateway that internally points to one of the four I2C peripherals
 *  after it was initialised. It can then be copied and passed around.
 *  Use an I2CHandle like this:
 *  
 *      // setup the configuration
 *      I2CHandle::Config i2c_conf;
 *      i2c_conf.periph = I2CHandle::Config::Peripheral::I2C_1;
 *      i2c_conf.speed  = I2CHandle::Config::Speed::I2C_400KHZ;
 *      i2c_conf.mode   = I2CHandle::Config::Mode::Master;
 *      i2c_conf.pin_config.scl  = {DSY_GPIOB, 8};
 *      i2c_conf.pin_config.sda  = {DSY_GPIOB, 9};
 *      // initialise the peripheral
 *      I2CHandle i2c;
 *      i2c.Init(i2c_conf);
 *      // now i2c points to the corresponding peripheral and can be used.
 *      i2c.TransmitBlocking( ... );
 */
class I2CHandle
{
  public:
    /** Contains settings for initialising an I2C interface. */
    struct Config
    {
        /** Specifies whether the interface will operate in master or slave mode. */
        enum class Mode
        {
            I2C_MASTER,
            I2C_SLAVE,
        };

        /** Specifices the internal peripheral to use (these are mapped to different pins on the hardware). */
        enum class Peripheral
        {
            I2C_1 = 0, /**< & */
            I2C_2,     /**< & */
            I2C_3,     /**< & */
            I2C_4,     /**< & */
        };

        /** Rate at which the clock/data will be sent/received. The device being used will have maximum speeds.
         *  1MHZ Mode is currently 886kHz
         */
        enum class Speed
        {
            I2C_100KHZ, /**< & */
            I2C_400KHZ, /**< & */
            I2C_1MHZ,   /**< & */
        };

        Peripheral periph; /**< & */
        struct
        {
            dsy_gpio_pin scl; /**< & */
            dsy_gpio_pin sda; /**< & */
        } pin_config;         /**< & */

        Speed speed; /**< & */
        Mode  mode;  /**< & */
        // 0x10 is chosen as a default to avoid conflicts with reserved addresses
        uint8_t address = 0x10; /**< & */
    };

    /** Return values for I2C functions. */
    enum class Result
    {
        OK, /**< & */
        ERR /**< & */
    };

    enum class Direction
    {
        TRANSMIT, /**< & */
        RECEIVE,  /**< & */
    };

    I2CHandle() : pimpl_(nullptr) {}
    I2CHandle(const I2CHandle& other) = default;
    I2CHandle& operator=(const I2CHandle& other) = default;

    /** Initializes an I2C peripheral. */
    Result Init(const Config& config);

    /** Returns the current config. */
    const Config& GetConfig() const;

    /** Transmits data and blocks until the transmission is complete.
     *  Use this for smaller transmissions of a few bytes.
     * 
     *  \param address      The slave device address. Unused in slave mode.
     *  \param data         A pointer to the data to be sent.
     *  \param size         The size of the data to be sent, in bytes.
     *  \param timeout      A timeout.
     */
    Result TransmitBlocking(uint16_t address,
                            uint8_t* data,
                            uint16_t size,
                            uint32_t timeout);

    /** Receives data and blocks until the reception is complete.
     *  Use this for smaller transmissions of a few bytes.
     * 
     *  \param address      The slave device address. Unused in slave mode.
     *  \param data         A pointer to the data to be received.
     *  \param size         The size of the data to be received, in bytes.
     *  \param timeout      A timeout.
     */
    Result ReceiveBlocking(uint16_t address,
                           uint8_t* data,
                           uint16_t size,
                           uint32_t timeout);

    /** A callback to be executed when a dma transfer is complete. */
    typedef void (*CallbackFunctionPtr)(void* context, Result result);

    /** Transmits data with a DMA and returns immediately. Use this for larger transmissions.
     *  The pointer to data must be located in the D2 memory domain by adding the 
     *  `DMA_BUFFER_MEM_SECTION` attribute like this:
     *      uint8_t DMA_BUFFER_MEM_SECTION my_buffer[100];
     *  If that is not possible for some reason, you MUST clear the cachelines spanning the size of 
     *  the buffer, before initiating the dma transfer by calling 
     *  `dsy_dma_clear_cache_for_buffer(buffer, size);`
     * 
     *  A single DMA is shared across I2C1, I2C2 and I2C3. I2C4 has no DMA support (yet).
     *  If the DMA is busy with another transfer, the job will be queued and executed later.
     *  If there is a job waiting to be executed for this I2C peripheral, this function
     *  will block until the queue is free and the job can be queued.
     * 
     *  \param address      The slave device address. Unused in slave mode.
     *  \param data         A pointer to the data to be sent.
     *  \param size         The size of the data to be sent, in bytes.
     *  \param callback     A callback to execute when the transfer finishes, or NULL.
     *  \param callback_context A pointer that will be passed back to you in the callback.      
     */
    Result TransmitDma(uint16_t            address,
                       uint8_t*            data,
                       uint16_t            size,
                       CallbackFunctionPtr callback,
                       void*               callback_context);

    /** Receives data with a DMA and returns immediately. Use this for larger transmissions.
     *  The pointer to data must be located in the D2 memory domain by adding the 
     *  `DMA_BUFFER_MEM_SECTION` attribute like this:
     *      uint8_t DMA_BUFFER_MEM_SECTION my_buffer[100];
     *  If that is not possible for some reason, you MUST clear the cachelines spanning the size of 
     *  the buffer, before initiating the dma transfer by calling 
     *  `dsy_dma_clear_cache_for_buffer(buffer, size);`
     * 
     *  A single DMA is shared across I2C, I2C2 and I2C3. I2C4 has no DMA support (yet).
     *  If the DMA is busy with another transfer, the job will be queued and executed later.
     *  If there is a job waiting to be executed for this I2C peripheral, this function
     *  will block until the queue is free and the job can be queued.
     * 
     *  \param address      The slave device address. Unused in slave mode.
     *  \param data         A pointer to the data buffer.
     *  \param size         The size of the data to be received, in bytes.
     *  \param callback     A callback to execute when the transfer finishes, or NULL.
     *  \param callback_context A pointer that will be passed back to you in the callback.      
     */
    Result ReceiveDma(uint16_t            address,
                      uint8_t*            data,
                      uint16_t            size,
                      CallbackFunctionPtr callback,
                      void*               callback_context);

    /** Reads an amount of data from a specific memory address. 
    *   This method will return an error if the I2C peripheral is in slave mode. 
    * 
    * \param address            The slave device address.
    * \param mem_address        Pointer to data containing the address to read from device.
    * \param mem_address_size   Size of the memory address in bytes.
    * \param data               Pointer to buffer that will be filled with contents at mem_address
    * \param data_size          Size of the data to be read in bytes.
    * \param timeout            The timeout in milliseconds before returning without communication
    */
    Result ReadDataAtAddress(uint16_t address,
                             uint16_t mem_address,
                             uint16_t mem_address_size,
                             uint8_t* data,
                             uint16_t data_size,
                             uint32_t timeout);

    /** Writes an amount of data from a specific memory address. 
    *   This method will return an error if the I2C peripheral is in slave mode. 
    * 
    * \param address            The slave device address.
    * \param mem_address        Pointer to data containing the address to write to device.
    * \param mem_address_size   Size of the memory address in bytes.
    * \param data               Pointer to buffer that will be written to the mem_address
    * \param data_size          Size of the data to be written in bytes.
    * \param timeout            The timeout in milliseconds before returning without communication
    */
    Result WriteDataAtAddress(uint16_t address,
                              uint16_t mem_address,
                              uint16_t mem_address_size,
                              uint8_t* data,
                              uint16_t data_size,
                              uint32_t timeout);


    class Impl; /**< & */

  private:
    Impl* pimpl_;
};

extern "C"
{
    /** internal. Used for global init. */
    void dsy_i2c_global_init();
};

} // namespace daisy