#include "daisy_seed.h"

extern "C"
{
#include "dev/codec_ak4556.h"
}

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

/** Vestigial function body for old function
 *  This is no longer in use.
 */
void DaisySeed::Configure() {}

void DaisySeed::Init(bool boost)
{
    //dsy_system_init();
    System::Config syscfg;
    boost ? syscfg.Boost() : syscfg.Defaults();

    ConfigureQspi();
    // Configure the built-in GPIOs.
    led.pin.port       = SEED_LED_PORT;
    led.pin.pin        = SEED_LED_PIN;
    led.mode           = DSY_GPIO_MODE_OUTPUT_PP;
    testpoint.pin.port = SEED_TEST_POINT_PORT;
    testpoint.pin.pin  = SEED_TEST_POINT_PIN;
    testpoint.mode     = DSY_GPIO_MODE_OUTPUT_PP;


    auto memory = System::GetProgramMemoryRegion();

    if(memory != System::MemoryRegion::INTERNAL_FLASH)
        syscfg.skip_clocks = true;

    system.Init(syscfg);

    if(memory != System::MemoryRegion::QSPI)
        qspi.Init(qspi_config);

    if(memory == System::MemoryRegion::INTERNAL_FLASH)
    {
        dsy_gpio_init(&led);
        dsy_gpio_init(&testpoint);
        sdram_handle.Init();
    }

    ConfigureAudio();

    callback_rate_ = AudioSampleRate() / AudioBlockSize();
    // Due to the added 16kB+ of flash usage,
    // and the fact that certain breakouts use
    // both; USB won't be initialized by the
    // SEED file.
    //usb_handle.Init(UsbHandle::FS_INTERNAL);
}

void DaisySeed::DeInit()
{
    // This is intended to be used by the bootloader, but
    // we don't want to reinitialize pretty much anything in the
    // target application, so...
    // qspi.DeInit();
    // sdram_handle.DeInit();
    // dsy_gpio_deinit(&led);
    // dsy_gpio_deinit(&testpoint);

    // dsy_gpio_pin codec_reset_pin;
    // codec_reset_pin = {DSY_GPIOB, 11};
    // // Perhaps a bit unnecessary, but maybe we'll make
    // // this non-static at some point
    // Ak4556::DeInit(codec_reset_pin);
    // audio_handle.DeInit();

    system.DeInit();
}

dsy_gpio_pin DaisySeed::GetPin(uint8_t pin_idx)
{
    dsy_gpio_pin p;
    pin_idx = pin_idx < 32 ? pin_idx : 0;
#ifndef SEED_REV2
    p = seedgpio[pin_idx];
#else
    p = {seed_ports[pin_idx], seed_pins[pin_idx]};
#endif
    return p;
}

void DaisySeed::DelayMs(size_t del)
{
    system.Delay(del);
}

void DaisySeed::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    audio_handle.Start(cb);
}

void DaisySeed::StartAudio(AudioHandle::AudioCallback cb)
{
    audio_handle.Start(cb);
}

void DaisySeed::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb)
{
    audio_handle.ChangeCallback(cb);
}

void DaisySeed::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    audio_handle.ChangeCallback(cb);
}

void DaisySeed::StopAudio()
{
    audio_handle.Stop();
}

void DaisySeed::SetAudioSampleRate(SaiHandle::Config::SampleRate samplerate)
{
    audio_handle.SetSampleRate(samplerate);
    callback_rate_ = AudioSampleRate() / AudioBlockSize();
}

float DaisySeed::AudioSampleRate()
{
    return audio_handle.GetSampleRate();
}

void DaisySeed::SetAudioBlockSize(size_t blocksize)
{
    audio_handle.SetBlockSize(blocksize);
    callback_rate_ = AudioSampleRate() / AudioBlockSize();
}

size_t DaisySeed::AudioBlockSize()
{
    return audio_handle.GetConfig().blocksize;
}

float DaisySeed::AudioCallbackRate() const
{
    return callback_rate_;
}

void DaisySeed::SetLed(bool state)
{
    dsy_gpio_write(&led, state);
}

void DaisySeed::SetTestPoint(bool state)
{
    dsy_gpio_write(&testpoint, state);
}

// Private Implementation

void DaisySeed::ConfigureQspi()
{
    qspi_config.device = QSPIHandle::Config::Device::IS25LP064A;
    qspi_config.mode   = QSPIHandle::Config::Mode::MEMORY_MAPPED;

    qspi_config.pin_config.io0 = dsy_pin(DSY_GPIOF, 8);
    qspi_config.pin_config.io1 = dsy_pin(DSY_GPIOF, 9);
    qspi_config.pin_config.io2 = dsy_pin(DSY_GPIOF, 7);
    qspi_config.pin_config.io3 = dsy_pin(DSY_GPIOF, 6);
    qspi_config.pin_config.clk = dsy_pin(DSY_GPIOF, 10);
    qspi_config.pin_config.ncs = dsy_pin(DSY_GPIOG, 6);
}
void DaisySeed::ConfigureAudio()
{
    // SAI1 -- Peripheral
    // Configure
    SaiHandle::Config sai_config;
    sai_config.periph          = SaiHandle::Config::Peripheral::SAI_1;
    sai_config.sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
    sai_config.bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
    sai_config.a_sync          = SaiHandle::Config::Sync::MASTER;
    sai_config.b_sync          = SaiHandle::Config::Sync::SLAVE;
    sai_config.pin_config.fs   = {DSY_GPIOE, 4};
    sai_config.pin_config.mclk = {DSY_GPIOE, 2};
    sai_config.pin_config.sck  = {DSY_GPIOE, 5};

    // Device-based Init
    switch(CheckBoardVersion())
    {
        case BoardVersion::DAISY_SEED_1_1:
        {
            // Data Line Directions
            sai_config.a_dir         = SaiHandle::Config::Direction::RECEIVE;
            sai_config.pin_config.sa = {DSY_GPIOE, 6};
            sai_config.b_dir         = SaiHandle::Config::Direction::TRANSMIT;
            sai_config.pin_config.sb = {DSY_GPIOE, 3};
            I2CHandle::Config i2c_config;
            i2c_config.mode           = I2CHandle::Config::Mode::I2C_MASTER;
            i2c_config.periph         = I2CHandle::Config::Peripheral::I2C_2;
            i2c_config.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
            i2c_config.pin_config.scl = {DSY_GPIOH, 4};
            i2c_config.pin_config.sda = {DSY_GPIOB, 11};
            I2CHandle i2c_handle;
            i2c_handle.Init(i2c_config);
            Wm8731::Config codec_cfg;
            codec_cfg.Defaults();
            Wm8731 codec;
            codec.Init(codec_cfg, i2c_handle);
        }
        break;
        case BoardVersion::DAISY_SEED:
        default:
        {
            // Data Line Directions
            sai_config.a_dir         = SaiHandle::Config::Direction::TRANSMIT;
            sai_config.pin_config.sa = {DSY_GPIOE, 6};
            sai_config.b_dir         = SaiHandle::Config::Direction::RECEIVE;
            sai_config.pin_config.sb = {DSY_GPIOE, 3};
            dsy_gpio_pin codec_reset_pin;
            codec_reset_pin = {DSY_GPIOB, 11};
            Ak4556::Init(codec_reset_pin);
        }
        break;
    }

    // Then Initialize
    SaiHandle sai_1_handle;
    sai_1_handle.Init(sai_config);

    // Audio
    AudioHandle::Config audio_config;
    audio_config.blocksize  = 48;
    audio_config.samplerate = SaiHandle::Config::SampleRate::SAI_48KHZ;
    audio_config.postgain   = 1.f;
    audio_handle.Init(audio_config, sai_1_handle);
}
void DaisySeed::ConfigureDac()
{
    // This would be the equivalent initialization as previously existed.
    // However, not all platforms have the DAC, and many use those pins
    // for other things.
    //    DacHandle::Config cfg;
    //    cfg.bitdepth   = DacHandle::Config::BitDepth::BITS_12;
    //    cfg.buff_state = DacHandle::Config::BufferState::ENABLED;
    //    cfg.mode       = DacHandle::Config::Mode::POLLING;
    //    cfg.chn        = DacHandle::Config::Channel::BOTH;
    //    dac.Init(cfg);
}

DaisySeed::BoardVersion DaisySeed::CheckBoardVersion()
{
    /** Version Checks:
     *  * Fall through is Daisy Seed v1 (aka Daisy Seed rev4)
     *  * PD3 tied to gnd is Daisy Seed v1.1 (aka Daisy Seed rev5)
     *  * PD4 tied to gnd reserved for future hardware
     */
    dsy_gpio pincheck;
    pincheck.mode = DSY_GPIO_MODE_INPUT;
    pincheck.pull = DSY_GPIO_PULLUP;
    pincheck.pin  = {DSY_GPIOD, 3};
    dsy_gpio_init(&pincheck);
    if(!dsy_gpio_read(&pincheck))
        return BoardVersion::DAISY_SEED_1_1;
    else
        return BoardVersion::DAISY_SEED;
}
