#include "daisy_seed.h"

using namespace daisy;

#define SEED_LED_PORT DSY_GPIOC
#define SEED_LED_PIN 7

#define SEED_TEST_POINT_PORT DSY_GPIOG
#define SEED_TEST_POINT_PIN 14

#ifndef SEED_REV2
const dsy_gpio_pin seedgpio[31] = {
    // GPIO 1-8
    //{DSY_GPIOA, 8}, // removed on Rev4
    {DSY_GPIOB, 12},
    {DSY_GPIOC, 11},
    {DSY_GPIOC, 10},
    {DSY_GPIOC, 9},
    {DSY_GPIOC, 8},
    {DSY_GPIOD, 2},
    {DSY_GPIOC, 12},
    // GPIO 9-16
    {DSY_GPIOG, 10},
    {DSY_GPIOG, 11},
    {DSY_GPIOB, 4},
    {DSY_GPIOB, 5},
    {DSY_GPIOB, 8},
    {DSY_GPIOB, 9},
    {DSY_GPIOB, 6},
    {DSY_GPIOB, 7},
    // GPIO 17-24
    {DSY_GPIOC, 0},
    {DSY_GPIOA, 3},
    {DSY_GPIOB, 1},
    {DSY_GPIOA, 7},
    {DSY_GPIOA, 6},
    {DSY_GPIOC, 1},
    {DSY_GPIOC, 4},
    {DSY_GPIOA, 5},
    // GPIO 17-24
    {DSY_GPIOA, 4},
    {DSY_GPIOA, 1},
    {DSY_GPIOA, 0},
    {DSY_GPIOD, 11},
    {DSY_GPIOG, 9},
    {DSY_GPIOA, 2},
    {DSY_GPIOB, 14},
    {DSY_GPIOB, 15},
};
#else
const dsy_gpio_port seed_ports[32] = {
    DSY_GPIOA, DSY_GPIOB, DSY_GPIOC, DSY_GPIOC, DSY_GPIOC, DSY_GPIOC, DSY_GPIOD,
    DSY_GPIOC, DSY_GPIOG, DSY_GPIOG, DSY_GPIOB, DSY_GPIOB, DSY_GPIOB, DSY_GPIOB,
    DSY_GPIOB, DSY_GPIOB, DSY_GPIOC, DSY_GPIOA, DSY_GPIOA, DSY_GPIOB, DSY_GPIOA,
    DSY_GPIOA, DSY_GPIOC, DSY_GPIOC, DSY_GPIOA, DSY_GPIOA, DSY_GPIOA, DSY_GPIOD,
    DSY_GPIOG, DSY_GPIOA, DSY_GPIOB, DSY_GPIOB,
};

const uint8_t seed_pins[32] = {
    8, 12, 11, 10, 9, 8, 7, 12, 10, 11, 4, 5,  8, 9, 6,  7,
    0, 1,  3,  1,  7, 6, 1, 5,  5,  4,  0, 11, 9, 2, 14, 15,
};

const dsy_gpio_pin seedgpio[32] = {
    {seed_ports[0], seed_pins[0]},   {seed_ports[1], seed_pins[1]},
    {seed_ports[2], seed_pins[2]},   {seed_ports[3], seed_pins[3]},
    {seed_ports[4], seed_pins[4]},   {seed_ports[5], seed_pins[5]},
    {seed_ports[6], seed_pins[6]},   {seed_ports[7], seed_pins[7]},
    {seed_ports[8], seed_pins[8]},   {seed_ports[9], seed_pins[9]},
    {seed_ports[10], seed_pins[10]}, {seed_ports[11], seed_pins[11]},
    {seed_ports[12], seed_pins[12]}, {seed_ports[13], seed_pins[13]},
    {seed_ports[14], seed_pins[14]}, {seed_ports[15], seed_pins[15]},
    {seed_ports[16], seed_pins[16]}, {seed_ports[17], seed_pins[17]},
    {seed_ports[18], seed_pins[18]}, {seed_ports[19], seed_pins[19]},
    {seed_ports[20], seed_pins[20]}, {seed_ports[21], seed_pins[21]},
    {seed_ports[22], seed_pins[22]}, {seed_ports[23], seed_pins[23]},
    {seed_ports[24], seed_pins[24]}, {seed_ports[25], seed_pins[25]},
    {seed_ports[26], seed_pins[26]}, {seed_ports[27], seed_pins[27]},
    {seed_ports[28], seed_pins[28]}, {seed_ports[29], seed_pins[29]},
    {seed_ports[30], seed_pins[30]}, {seed_ports[31], seed_pins[31]},
};
#endif

// Public Initialization

void DaisySeed::Configure()
{
    // Configure internal peripherals
    ConfigureSdram();
    ConfigureQspi();
    ConfigureAudio();
    ConfigureDac();
    //ConfigureI2c();
    // Configure the built-in GPIOs.
    led_.pin.port       = SEED_LED_PORT;
    led_.pin.pin        = SEED_LED_PIN;
    led_.mode           = DSY_GPIO_MODE_OUTPUT_PP;
    testpoint_.pin.port = SEED_TEST_POINT_PORT;
    testpoint_.pin.pin  = SEED_TEST_POINT_PIN;
    testpoint_.mode     = DSY_GPIO_MODE_OUTPUT_PP;
}

void DaisySeed::Init()
{
    dsy_system_init();
    dsy_sdram_init(&sdram_handle);
    dsy_qspi_init(&qspi_handle);
    dsy_gpio_init(&led_);
    dsy_gpio_init(&testpoint_);
    dsy_audio_init(&audio_handle);
    dsy_tim_init();
    dsy_tim_start();
    // Due to the added 16kB+ of flash usage,
    // and the fact that certain breakouts use
    // both; USB won't be initialized by the
    // SEED file.
    //usb_handle.Init(UsbHandle::FS_INTERNAL);
}
dsy_gpio_pin DaisySeed::GetPin(uint8_t pin_idx)
{
    dsy_gpio_pin p;
    pin_idx = pin_idx < 32 ? pin_idx : 0;
#ifndef SEED_REV2
    p = seedgpio[pin_idx];
#else
    p                            = {seed_ports[pin_idx], seed_pins[pin_idx]};
#endif
    return p;
}

void DaisySeed::StartAudio(dsy_audio_callback cb)
{
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, cb);
    dsy_audio_start(DSY_AUDIO_INTERNAL);
}

void DaisySeed::StartAudio(dsy_audio_mc_callback cb)
{
    dsy_audio_set_mc_callback(cb);
    dsy_audio_start(DSY_AUDIO_INTERNAL);
}

void DaisySeed::ChangeAudioCallback(dsy_audio_callback cb)
{
    dsy_audio_set_callback(DSY_AUDIO_INTERNAL, cb);
}

void DaisySeed::ChangeAudioCallback(dsy_audio_mc_callback cb)
{
    dsy_audio_set_mc_callback(cb);
}

float DaisySeed::AudioSampleRate()
{
    // TODO fix to get this from configured rate.
    return DSY_AUDIO_SAMPLE_RATE;
}

void DaisySeed::SetAudioBlockSize(size_t blocksize)
{
    dsy_audio_set_blocksize(DSY_AUDIO_INTERNAL, blocksize);
}

void DaisySeed::SetLed(bool state)
{
    dsy_gpio_write(&led_, state);
}
void DaisySeed::SetTestPoint(bool state)
{
    dsy_gpio_write(&testpoint_, state);
}

// Private Implementation

void DaisySeed::ConfigureSdram()
{
    dsy_gpio_pin *pin_group;
    sdram_handle.state             = DSY_SDRAM_STATE_ENABLE;
    pin_group                      = sdram_handle.pin_config;
    pin_group[DSY_SDRAM_PIN_SDNWE] = dsy_pin(DSY_GPIOH, 5);
}
void DaisySeed::ConfigureQspi()
{
    dsy_gpio_pin *pin_group;
    qspi_handle.device = DSY_QSPI_DEVICE_IS25LP064A;
    qspi_handle.mode   = DSY_QSPI_MODE_DSY_MEMORY_MAPPED;
    pin_group          = qspi_handle.pin_config;


    pin_group[DSY_QSPI_PIN_IO0] = dsy_pin(DSY_GPIOF, 8);
    pin_group[DSY_QSPI_PIN_IO1] = dsy_pin(DSY_GPIOF, 9);
    pin_group[DSY_QSPI_PIN_IO2] = dsy_pin(DSY_GPIOF, 7);
    pin_group[DSY_QSPI_PIN_IO3] = dsy_pin(DSY_GPIOF, 6);
    pin_group[DSY_QSPI_PIN_CLK] = dsy_pin(DSY_GPIOF, 10);
    pin_group[DSY_QSPI_PIN_NCS] = dsy_pin(DSY_GPIOG, 6);
}
void DaisySeed::ConfigureAudio()
{
    dsy_gpio_pin *pin_group;
    sai_handle.init = DSY_AUDIO_INIT_SAI1;
    // SAI1 - config
#ifndef SEED_REV2
    sai_handle.device[DSY_SAI_1]      = DSY_AUDIO_DEVICE_AK4556;
    sai_handle.samplerate[DSY_SAI_1]  = DSY_AUDIO_SAMPLERATE_48K;
    sai_handle.bitdepth[DSY_SAI_1]    = DSY_AUDIO_BITDEPTH_24;
    sai_handle.a_direction[DSY_SAI_1] = DSY_AUDIO_TX;
    sai_handle.b_direction[DSY_SAI_1] = DSY_AUDIO_RX;
#else
    sai_handle.device[DSY_SAI_1] = DSY_AUDIO_DEVICE_WM8731;
    sai_handle.samplerate[DSY_SAI_1]  = DSY_AUDIO_SAMPLERATE_48K;
    sai_handle.bitdepth[DSY_SAI_1]    = DSY_AUDIO_BITDEPTH_16;
    sai_handle.a_direction[DSY_SAI_1] = DSY_AUDIO_RX;
    sai_handle.b_direction[DSY_SAI_1] = DSY_AUDIO_TX;
#endif
    sai_handle.sync_config[DSY_SAI_1] = DSY_AUDIO_SYNC_MASTER;
    pin_group                         = sai_handle.sai1_pin_config;

    pin_group[DSY_SAI_PIN_MCLK] = dsy_pin(DSY_GPIOE, 2);
    pin_group[DSY_SAI_PIN_FS]   = dsy_pin(DSY_GPIOE, 4);
    pin_group[DSY_SAI_PIN_SCK]  = dsy_pin(DSY_GPIOE, 5);
    pin_group[DSY_SAI_PIN_SIN]  = dsy_pin(DSY_GPIOE, 6);
    pin_group[DSY_SAI_PIN_SOUT] = dsy_pin(DSY_GPIOE, 3);

    // SAI2 - config
    // Example Config
    //    sai_handle.device[DSY_SAI_2]      = DSY_AUDIO_DEVICE_WM8731;
    //    sai_handle.samplerate[DSY_SAI_2]  = DSY_AUDIO_SAMPLERATE_48K;
    //    sai_handle.bitdepth[DSY_SAI_2]    = DSY_AUDIO_BITDEPTH_16;
    //    sai_handle.sync_config[DSY_SAI_2] = DSY_AUDIO_SYNC_SLAVE;
    pin_group = sai_handle.sai2_pin_config;

    pin_group[DSY_SAI_PIN_MCLK] = dsy_pin(DSY_GPIOA, 1);
    pin_group[DSY_SAI_PIN_FS]   = dsy_pin(DSY_GPIOG, 9);
    pin_group[DSY_SAI_PIN_SCK]  = dsy_pin(DSY_GPIOA, 2);
    pin_group[DSY_SAI_PIN_SIN]  = dsy_pin(DSY_GPIOD, 11);
    pin_group[DSY_SAI_PIN_SOUT] = dsy_pin(DSY_GPIOA, 0);

    audio_handle.sai = &sai_handle;
    //audio_handle.dev0_i2c   = NULL;
    //audio_handle.dev1_i2c   = NULL;
    audio_handle.block_size = 48;
    dsy_audio_set_blocksize(DSY_AUDIO_INTERNAL, 48);
    dsy_audio_set_blocksize(DSY_AUDIO_EXTERNAL, 48);
}
void DaisySeed::ConfigureDac()
{
    dsy_gpio_pin *pin_group;
    dac_handle.mode         = DSY_DAC_MODE_POLLING;
    dac_handle.bitdepth     = DSY_DAC_BITS_12;
    pin_group               = dac_handle.pin_config;
    pin_group[DSY_DAC_CHN1] = dsy_pin(DSY_GPIOA, 4);
    pin_group[DSY_DAC_CHN2] = dsy_pin(DSY_GPIOA, 5);
}
/*void DaisySeed::ConfigureI2c()
{
    dsy_gpio_pin *pin_group;
    // TODO: Add Config for I2C3 and I2C4
    // I2C 1 - (On daisy patch this controls the LED Driver, and the WM8731).
    i2c1_handle.periph              = DSY_I2C_PERIPH_1;
    i2c1_handle.speed               = DSY_I2C_SPEED_400KHZ;
    pin_group                       = i2c1_handle.pin_config;
    pin_group[DSY_I2C_PIN_SCL].port = DSY_GPIOB;
    pin_group[DSY_I2C_PIN_SCL].pin  = 8;
    pin_group[DSY_I2C_PIN_SDA].port = DSY_GPIOB;
    pin_group[DSY_I2C_PIN_SDA].pin  = 9;
    // I2C 2 - (On daisy patch this controls the on-board WM8731)
    i2c2_handle.periph              = DSY_I2C_PERIPH_2;
    i2c2_handle.speed               = DSY_I2C_SPEED_400KHZ;
    pin_group                       = i2c2_handle.pin_config;
    pin_group[DSY_I2C_PIN_SCL].port = DSY_GPIOH;
    pin_group[DSY_I2C_PIN_SCL].pin  = 4;
    pin_group[DSY_I2C_PIN_SDA].port = DSY_GPIOB;
    pin_group[DSY_I2C_PIN_SDA].pin  = 11;
}*/
