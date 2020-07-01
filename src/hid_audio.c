#include <string.h>
#include "daisy.h" // todo figure out what to do about this.
#include "daisy_core.h"
#include "hid_audio.h"
#include "dev_codec_pcm3060.h"
#include "dev_codec_wm8731.h"
#include "dev_codec_ak4556.h"
#include "stm32h7xx_hal.h"
#include "sys_dma.h"
#include "util_hal_map.h"

#define DSY_AUDIO_DMA_BUFFER_SIZE_MAX \
    (DSY_AUDIO_BLOCK_SIZE_MAX * DSY_AUDIO_CHANNELS_MAX * 2)
#define DSY_DMA_BUFFER_SECTOR __attribute__((section(".sram1_bss")))

#define DSY_PROFILE_AUDIO_CALLBACK 1

extern SAI_HandleTypeDef hsai_BlockA1;
extern SAI_HandleTypeDef hsai_BlockB1;
extern SAI_HandleTypeDef hsai_BlockA2;
extern SAI_HandleTypeDef hsai_BlockB2;

static uint8_t using_new_callbacks;

// Define/Declare global audio structure.
typedef struct
{
    dsy_audio_callback    callback;
    dsy_audio_mc_callback mc_callback;
    int32_t*              dma_buffer_rx;
    int32_t*              dma_buffer_tx;
    float                 in[DSY_AUDIO_BLOCK_SIZE_MAX * DSY_AUDIO_CHANNELS_MAX];
    float             out[DSY_AUDIO_BLOCK_SIZE_MAX * DSY_AUDIO_CHANNELS_MAX];
    size_t            block_size, offset, dma_size;
    uint8_t           bitdepth, device, channels;
    dsy_i2c_handle*   device_control_hi2c;
    dsy_audio_handle* config_handle;
} dsy_audio;

//  Static Buffers in non-cached SRAM1 for DMA
static int32_t DSY_DMA_BUFFER_SECTOR
               sai1_dma_buffer_rx[DSY_AUDIO_DMA_BUFFER_SIZE_MAX];
static int32_t DSY_DMA_BUFFER_SECTOR
               sai1_dma_buffer_tx[DSY_AUDIO_DMA_BUFFER_SIZE_MAX];
static int32_t DSY_DMA_BUFFER_SECTOR
               sai2_dma_buffer_rx[DSY_AUDIO_DMA_BUFFER_SIZE_MAX];
static int32_t DSY_DMA_BUFFER_SECTOR
               sai2_dma_buffer_tx[DSY_AUDIO_DMA_BUFFER_SIZE_MAX];

// Initialize Gate Output GPIO (only for timing in this case)
#ifdef DSY_PROFILE_AUDIO_CALLBACK
#define PROFILE_GPIO_PIN GPIO_PIN_14
#define PROFILE_GPIO_PORT GPIOG
static void init_gpio()
{
    GPIO_InitTypeDef ginit;
    ginit.Pin   = PROFILE_GPIO_PIN;
    ginit.Mode  = GPIO_MODE_OUTPUT_PP;
    ginit.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(PROFILE_GPIO_PORT, &ginit);
}
#endif


static dsy_audio audio_handle;
static dsy_audio audio_handle_ext;

static dsy_audio* get_audio_from_sai(SAI_HandleTypeDef* hsai)
{
    return (hsai->Instance == SAI1_Block_A || hsai->Instance == SAI1_Block_B)
               ? &audio_handle
               : &audio_handle_ext;
}

extern DMA_HandleTypeDef hdma_sai1_a;
extern DMA_HandleTypeDef hdma_sai1_b;
extern DMA_HandleTypeDef hdma_sai2_a;
extern DMA_HandleTypeDef hdma_sai2_b;

void DMA1_Stream0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_sai1_a);
}

void DMA1_Stream1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_sai1_b);
}

void DMA1_Stream3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_sai2_a);
}
void DMA1_Stream4_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_sai2_b);
}


void dsy_audio_passthru(float* in, float* out, size_t size)
{
    memcpy(out, in, sizeof(float) * size);
}

void dsy_audio_silence(float* in, float* out, size_t size)
{
    memset(out, 0, sizeof(float) * size);
}

// TODO: fix I2C to be compliant with the new model.
void dsy_audio_init(dsy_audio_handle* handle)
{
    uint8_t dev0, dev1, intext;
    audio_handle.config_handle     = handle;
    audio_handle_ext.config_handle = handle;
    intext                         = handle->sai->init;
    dev0                           = handle->sai->device[DSY_SAI_1];
    dev1                           = handle->sai->device[DSY_SAI_2];
    audio_handle.block_size = handle->block_size <= DSY_AUDIO_BLOCK_SIZE_MAX
                                  ? handle->block_size
                                  : DSY_AUDIO_BLOCK_SIZE_MAX;
    audio_handle_ext.block_size = handle->block_size <= DSY_AUDIO_BLOCK_SIZE_MAX
                                      ? handle->block_size
                                      : DSY_AUDIO_BLOCK_SIZE_MAX;
    audio_handle.bitdepth     = handle->sai->bitdepth[DSY_SAI_1];
    audio_handle_ext.bitdepth = handle->sai->bitdepth[DSY_SAI_2];
    audio_handle.callback     = dsy_audio_passthru;
    audio_handle_ext.callback = dsy_audio_passthru;
    dsy_sai_init_from_handle(handle->sai);
    if(handle->dev0_i2c != NULL)
        dsy_i2c_init(handle->dev0_i2c);
    if(handle->dev1_i2c != NULL)
        dsy_i2c_init(handle->dev1_i2c);
    audio_handle.dma_buffer_rx     = sai1_dma_buffer_rx;
    audio_handle.dma_buffer_tx     = sai1_dma_buffer_tx;
    audio_handle_ext.dma_buffer_rx = sai2_dma_buffer_rx;
    audio_handle_ext.dma_buffer_tx = sai2_dma_buffer_tx;
    audio_handle.channels          = 2;
    audio_handle_ext.channels      = 2;
    audio_handle.dma_size = audio_handle.block_size * audio_handle.channels * 2;
    audio_handle_ext.dma_size
        = audio_handle_ext.block_size * audio_handle_ext.channels * 2;
    if(intext == DSY_AUDIO_INIT_SAI1 || intext == DSY_AUDIO_INIT_BOTH)
    {
        audio_handle.device = dev0;
        uint8_t mcu_is_master
            = handle->sai->sync_config[DSY_SAI_1] == DSY_AUDIO_SYNC_MASTER ? 1
                                                                           : 0;
        switch(dev0)
        {
            case DSY_AUDIO_DEVICE_WM8731:
                codec_wm8731_init(
                    handle->dev0_i2c, mcu_is_master, 48000.0f, 16);
                break;
            case DSY_AUDIO_DEVICE_PCM3060:
                codec_pcm3060_init(handle->dev0_i2c);
                break;
            case DSY_AUDIO_DEVICE_AK4556:
                // Reset pin on board is PB11
                {
                    dsy_gpio_pin rpin;
                    rpin.port = DSY_GPIOB;
                    rpin.pin  = 11;
                    codec_ak4556_init(rpin);
                }
                break;
            default: break;
        }
        for(size_t i = 0; i < DSY_AUDIO_DMA_BUFFER_SIZE_MAX; i++)
        {
            audio_handle.dma_buffer_rx[i] = 0;
            audio_handle.dma_buffer_tx[i] = 0;
        }
        for(size_t i = 0; i < DSY_AUDIO_BLOCK_SIZE_MAX; i++)
        {
            audio_handle.in[i]  = 0.0f;
            audio_handle.out[i] = 0.0f;
        }
        audio_handle.offset = 0;
    }
    if(intext == DSY_AUDIO_INIT_SAI2 || intext == DSY_AUDIO_INIT_BOTH)
    {
        audio_handle_ext.device = dev1;
        uint8_t mcu_is_master
            = handle->sai->sync_config[DSY_SAI_2] == DSY_AUDIO_SYNC_MASTER ? 1
                                                                           : 0;
        switch(dev1)
        {
            case DSY_AUDIO_DEVICE_WM8731:
                codec_wm8731_init(
                    handle->dev1_i2c, mcu_is_master, 48000.0f, 16);
                break;
            case DSY_AUDIO_DEVICE_PCM3060:
                codec_pcm3060_init(handle->dev1_i2c);
                break;
            case DSY_AUDIO_DEVICE_AK4556:
                // Reset pin on board is PB11
                {
                    //					// Figure out how we want to support passing in the pin for this.
                    //					dsy_gpio_pin rpin;
                    //					rpin = {DSY_GPIOB, 11};
                    //					codec_ak4556_init(rpin);
                }
                break;
            default: break;
        }
        for(size_t i = 0; i < DSY_AUDIO_DMA_BUFFER_SIZE_MAX; i++)
        {
            audio_handle_ext.dma_buffer_rx[i] = 0;
            audio_handle_ext.dma_buffer_tx[i] = 0;
        }
        for(size_t i = 0; i < DSY_AUDIO_BLOCK_SIZE_MAX; i++)
        {
            audio_handle_ext.in[i]  = 0.0f;
            audio_handle_ext.out[i] = 0.0f;
        }
        audio_handle.offset = 0;
    }
    if(intext == DSY_AUDIO_INIT_BOTH)
    {
        using_new_callbacks = 1;
    }
    else
    {
        using_new_callbacks = 0;
    }

#ifdef DSY_PROFILE_AUDIO_CALLBACK
    init_gpio();
#endif
#ifdef __USBD_AUDIO_IF_H__
    audio_start(); // start audio callbacks, and then we'll see if it works or not...
#endif
}

void dsy_audio_set_callback(uint8_t intext, dsy_audio_callback cb)
{
    using_new_callbacks = 0;
    if(intext == DSY_AUDIO_INTERNAL)
    {
        audio_handle.callback = cb;
    }
    else
    {
        audio_handle_ext.callback = cb;
    }
}

void dsy_audio_set_mc_callback(dsy_audio_mc_callback cb)
{
    using_new_callbacks      = 1;
    audio_handle.mc_callback = cb;
}

void dsy_audio_set_blocksize(uint8_t intext, size_t blocksize)
{
    if(intext == DSY_AUDIO_INTERNAL)
    {
        audio_handle.block_size = blocksize <= DSY_AUDIO_BLOCK_SIZE_MAX
                                      ? blocksize
                                      : DSY_AUDIO_BLOCK_SIZE_MAX;

        audio_handle.dma_size
            = audio_handle.block_size * audio_handle.channels * 2;
    }
    else
    {
        audio_handle_ext.block_size = blocksize <= DSY_AUDIO_BLOCK_SIZE_MAX
                                          ? blocksize
                                          : DSY_AUDIO_BLOCK_SIZE_MAX;
        audio_handle_ext.dma_size
            = audio_handle_ext.block_size * audio_handle_ext.channels * 2;
    }
}

void dsy_audio_start(uint8_t intext)
{
    if(intext == DSY_AUDIO_INTERNAL)
    {
        if(audio_handle.config_handle->sai->a_direction[DSY_SAI_1]
           == DSY_AUDIO_RX)
        {
            HAL_SAI_Receive_DMA(&hsai_BlockA1,
                                (uint8_t*)audio_handle.dma_buffer_rx,
                                audio_handle.dma_size);
        }
        else
        {
            HAL_SAI_Transmit_DMA(&hsai_BlockA1,
                                 (uint8_t*)audio_handle.dma_buffer_tx,
                                 audio_handle.dma_size);
        }
        if(audio_handle.config_handle->sai->b_direction[DSY_SAI_1]
           == DSY_AUDIO_RX)
        {
            HAL_SAI_Receive_DMA(&hsai_BlockB1,
                                (uint8_t*)audio_handle.dma_buffer_rx,
                                audio_handle.dma_size);
        }
        else
        {
            HAL_SAI_Transmit_DMA(&hsai_BlockB1,
                                 (uint8_t*)audio_handle.dma_buffer_tx,
                                 audio_handle.dma_size);
        }
    }
    else
    {
        if(audio_handle_ext.config_handle->sai->a_direction[DSY_SAI_2]
           == DSY_AUDIO_RX)
        {
            HAL_SAI_Receive_DMA(&hsai_BlockA2,
                                (uint8_t*)audio_handle_ext.dma_buffer_rx,
                                audio_handle_ext.dma_size);
        }
        else
        {
            HAL_SAI_Transmit_DMA(&hsai_BlockA2,
                                 (uint8_t*)audio_handle_ext.dma_buffer_tx,
                                 audio_handle_ext.dma_size);
        }
        if(audio_handle_ext.config_handle->sai->b_direction[DSY_SAI_2]
           == DSY_AUDIO_RX)
        {
            HAL_SAI_Receive_DMA(&hsai_BlockB2,
                                (uint8_t*)audio_handle_ext.dma_buffer_rx,
                                audio_handle_ext.dma_size);
        }
        else
        {
            HAL_SAI_Transmit_DMA(&hsai_BlockB2,
                                 (uint8_t*)audio_handle_ext.dma_buffer_tx,
                                 audio_handle_ext.dma_size);
        }
    }
}

void audio_stop(uint8_t intext)
{
    if(intext == DSY_AUDIO_INTERNAL)
    {
        HAL_SAI_DMAStop(&hsai_BlockA1);
        HAL_SAI_DMAStop(&hsai_BlockB1);
    }
    else
    {
        HAL_SAI_DMAStop(&hsai_BlockA2);
        HAL_SAI_DMAStop(&hsai_BlockB2);
    }
}

// If the device supports hardware bypass, enter that mode.
void dsy_audio_enter_bypass(uint8_t intext)
{
    if(intext == DSY_AUDIO_INTERNAL)
    {
        switch(audio_handle.device)
        {
            case DSY_AUDIO_DEVICE_WM8731:
                codec_wm8731_enter_bypass(audio_handle.device_control_hi2c);
                break;
            default: break;
        }
    }
}

// If the device supports hardware bypass, exit that mode.
void dsy_audio_exit_bypass(uint8_t intext)
{
    if(intext == DSY_AUDIO_INTERNAL)
    {
        switch(audio_handle.device)
        {
            case DSY_AUDIO_DEVICE_WM8731:
                // TODO: Fix this, it doesn't work
                codec_wm8731_exit_bypass(audio_handle.device_control_hi2c);
                break;
            default: break;
        }
    }
}
// Static Functions Below

static void internal_callback(SAI_HandleTypeDef* hsai, size_t offset)
{
#ifdef DSY_PROFILE_AUDIO_CALLBACK
    HAL_GPIO_WritePin(PROFILE_GPIO_PORT, PROFILE_GPIO_PIN, 1);
#endif
    dsy_audio* ah = get_audio_from_sai(hsai);

    const int32_t* ini = ah->dma_buffer_rx + offset;
    float*         inf = ah->in;
    //const float*   endi = ah->in + ah->block_size;
    const float* endi = (ah->in + ah->dma_size / 2);
    if(ah->bitdepth == DSY_AUDIO_BITDEPTH_24)
    {
        while(inf != endi)
        {
            //            *inf++ = s242f((*ini++) << 2);
            *inf++ = s242f(*ini++);
        }
    }
    else
    {
        while(inf != endi)
        {
            *inf++ = s162f(*ini++);
        }
    }

    ah->callback(ah->in, ah->out, ah->block_size * ah->channels);
    int32_t*     outi = ah->dma_buffer_tx + offset;
    const float* outf = ah->out;
    const float* endo = ah->out + (ah->dma_size / 2);
    if(ah->bitdepth == DSY_AUDIO_BITDEPTH_24)
    {
        while(outf != endo)
        {
            *outi++ = f2s24(*outf++);
        }
    }
    else
    {
        while(outf != endo)
        {
            *outi++ = f2s16(*outf++);
        }
    }
#ifdef DSY_PROFILE_AUDIO_CALLBACK
    HAL_GPIO_WritePin(PROFILE_GPIO_PORT, PROFILE_GPIO_PIN, 0);
#endif
}

void test_mc_callback(float** in, float** out, size_t size)
{
    for(size_t i = 0; i < 4; i++)
    {
        for(size_t j = 0; j < size; j++)
        {
            out[i][j] = in[i][j];
        }
    }
}

static void combined_callback(size_t offset)
{
    const size_t size = audio_handle.block_size;
    float*       outf[4];
    float*       inf[4];
    inf[0]  = audio_handle.in;
    inf[1]  = audio_handle.in + size;
    inf[2]  = audio_handle_ext.in;
    inf[3]  = audio_handle_ext.in + size;
    outf[0] = audio_handle.out;
    outf[1] = audio_handle.out + size;
    outf[2] = audio_handle_ext.out;
    outf[3] = audio_handle_ext.out + size;
    // Deinterleave Inputs
    const int32_t *ini_a, *ini_b;
    ini_a = audio_handle.dma_buffer_rx + offset;
    ini_b = audio_handle_ext.dma_buffer_rx + offset;
    switch(audio_handle.bitdepth)
    {
        case DSY_AUDIO_BITDEPTH_24:
            for(size_t i = 0; i < size; i++)
            {
                inf[0][i] = s242f(ini_a[(i * 2)]);
                inf[1][i] = s242f(ini_a[(i * 2) + 1]);
                inf[2][i] = s242f(ini_b[(i * 2)]);
                inf[3][i] = s242f(ini_b[(i * 2) + 1]);
            }
            break;
        case DSY_AUDIO_BITDEPTH_16:
            for(size_t i = 0; i < size; i++)
            {
                inf[0][i] = s162f(ini_a[(i * 2)]);
                inf[1][i] = s162f(ini_a[(i * 2) + 1]);
                inf[2][i] = s162f(ini_b[(i * 2)]);
                inf[3][i] = s162f(ini_b[(i * 2) + 1]);
            }
            break;
        default: break;
    }

    // Process
    audio_handle.mc_callback((float**)inf, (float**)outf, size);

    // Interleave Output Buffers
    int32_t *outi_a, *outi_b;
    outi_a = audio_handle.dma_buffer_tx + offset;
    outi_b = audio_handle_ext.dma_buffer_tx + offset;
    switch(audio_handle.bitdepth)
    {
        case DSY_AUDIO_BITDEPTH_24:
            for(size_t i = 0; i < size; i++)
            {
                outi_a[(i * 2)]     = f2s24(outf[0][i]);
                outi_a[(i * 2) + 1] = f2s24(outf[1][i]);
                outi_b[(i * 2)]     = f2s24(outf[2][i]);
                outi_b[(i * 2) + 1] = f2s24(outf[3][i]);
            }
            break;
        case DSY_AUDIO_BITDEPTH_16:
            for(size_t i = 0; i < size; i++)
            {
                outi_a[(i * 2)]     = f2s24(outf[0][i]);
                outi_a[(i * 2) + 1] = f2s24(outf[1][i]);
                outi_b[(i * 2)]     = f2s24(outf[2][i]);
                outi_b[(i * 2) + 1] = f2s24(outf[3][i]);
            }
            break;
        default: break;
    }
}


// DMA Callbacks
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef* hsai)
{
    if(using_new_callbacks == 1)
    {
        if(hsai->Instance == SAI1_Block_A || hsai->Instance == SAI1_Block_B)
            combined_callback(0);
    }
    else
    {
        internal_callback(hsai, 0);
    }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef* hsai)
{
    if(using_new_callbacks == 1)
    {
        if(hsai->Instance == SAI1_Block_A || hsai->Instance == SAI1_Block_B)
            combined_callback(audio_handle.dma_size / 2);
    }
    else
    {
        internal_callback(hsai, audio_handle.dma_size / 2);
    }
}
//void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef* hsai)
//{
//    internal_callback(hsai, 0);
//}
//
//void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef* hsai)
//{
//    internal_callback(hsai, audio_handle.dma_size / 2);
//}
