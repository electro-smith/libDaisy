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
 *      i2c_conf.periph = I2CHandle::Config::Peripheral::I2C1;
 *      i2c_conf.speed  = I2CHandle::Config::Speed::I2C_400KHZ;
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
        Speed speed;          /**< & */
    };

    /** Return values for I2C functions. */
    enum class Result
    {
        OK, /**< & */
        ERR /**< & */
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
     *  \param address      The slave device address.
     *  \param data         A pointer to the data to be sent.
     *  \param size         The size of the data to be sent, in bytes.
     *  \param timeout      A timeout.
     */
    Result TransmitBlocking(uint16_t address,
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
     *  A single DMA is shared across I2C, I2C2 and I2C3. I2C4 has no DMA support (yet).
     *  If the DMA is busy with another transfer, the job will be queued and executed later.
     *  If there is a job waiting to be executed for this I2C peripheral, this function
     *  will block until the queue is free and the job can be queued.
     * 
     *  \param address      The slave device address.
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

#if 0


/* TODO:
- Add DMA support
- Add timing calc based on current clock source freq.
- Add discrete rx/tx functions (currently other drivers still need to call ST HAL functions).
*/

/* Errata:
- 1MHZ (FastMode+) is currently only 886kHZ (should get remeasured with latest clock tree). */
#ifndef DSY_I2C_H
#define DSY_I2C_H /**< Macro */
#ifdef __cplusplus
extern "C"
{
#endif

#include "daisy_core.h"

    /** @addtogroup serial
    @{
    */

    /** Driver for controlling I2C devices.
     * TODO: Add DMA support for I2C4
     * TODO: Add support for receive transmissions.
     */

    /** Specifices the internal peripheral to use (these are mapped to different pins on the hardware). */
    typedef enum
    {
        DSY_I2C_PERIPH_1 = 0, /**< & */
        DSY_I2C_PERIPH_2,     /**< & */
        DSY_I2C_PERIPH_3,     /**< & */
        DSY_I2C_PERIPH_4,     /**< & */
    } dsy_i2c_periph;

    /** Rate at which the clock/data will be sent/received. The device being used will have maximum speeds.
     *  1MHZ Mode is currently 886kHz**
     */
    typedef enum
    {
        DSY_I2C_SPEED_100KHZ, /**< & */
        DSY_I2C_SPEED_400KHZ, /**< & */
        DSY_I2C_SPEED_1MHZ,   /**< & */
        DSY_I2C_SPEED_LAST,   /**< & */
    } dsy_i2c_speed;

    /** Contains settings for initialising an I2C interface, and can be passed to dev_ drivers that require I2C. */
    typedef struct
    {
        dsy_i2c_periph periph; /**< & */
        struct
        {
            dsy_gpio_pin scl; /**< & */
            dsy_gpio_pin sda; /**< & */
        } pin_config;         /**< & */
        dsy_i2c_speed speed;  /**< & */
    } dsy_i2c_config;

    /** Return values for I2C functions. */
    typedef enum
    {
        DSY_I2C_RES_OK,  /**< & */
        DSY_I2C_RES_ERR, /**< & */
    } dsy_i2c_result;

    /** A handle used to refer to an I2C peripheral. */
    typedef struct
    {
        dsy_i2c_config config;
        void*          hal_hi2c;
    } dsy_i2c_handle;

    /** Initializes an I2C peripheral with the data given from the config. */
    dsy_i2c_result dsy_i2c_init(dsy_i2c_handle* dsy_hi2c);

    /** Transmits data and blocks until the transmission is complete.
     *  Use this for smaller transmissions of a few bytes.
     * 
     *  \param dsy_hi2c     The I2C peripheral to use.
     *  \param address      The slave device address.
     *  \param data         A pointer to the data to be sent.
     *  \param size         The size of the data to be sent, in bytes.
     *  \param timeout      A timeout.
     */
    dsy_i2c_result dsy_i2c_transmit_blocking(dsy_i2c_handle* dsy_hi2c,
                                             uint16_t        address,
                                             uint8_t*        data,
                                             uint16_t        size,
                                             uint32_t        timeout);

    /** A callback to be executed when a dma transfer is complete. */
    typedef void (*dsy_i2c_transf_cplt_callback_t)(void*          context,
                                                   dsy_i2c_result result);

    /** Transmits data with a DMA and returns immediately. Use this for larger transmissions.
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
     *  \param dsy_hi2c     The I2C peripheral to use.
     *  \param address      The slave device address.
     *  \param data         A pointer to the data to be sent.
     *  \param size         The size of the data to be sent, in bytes.
     *  \param callback     A callback to execute when the transfer finishes, or NULL.
     *  \param callback_context A pointer that will be passed back to you in the callback.      
     */
    dsy_i2c_result dsy_i2c_transmit_dma(dsy_i2c_handle*                dsy_hi2c,
                                        uint16_t                       address,
                                        uint8_t*                       data,
                                        uint16_t                       size,
                                        dsy_i2c_transf_cplt_callback_t callback,
                                        void* callback_context);

    /** Returns DSY_I2C_RES_OK if the peripheral is ready to transfer data. */
    dsy_i2c_result dsy_i2c_ready(dsy_i2c_handle* dsy_hi2c);

#ifdef __cplusplus
}
#endif
#endif /*__ i2c_H */
/** @} */
#endif