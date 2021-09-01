#include "daisy_patch_sm.h"

#define PIN_ADC_CTRL_1 \
    {                  \
        DSY_GPIOA, 3   \
    }
#define PIN_ADC_CTRL_2 \
    {                  \
        DSY_GPIOA, 6   \
    }
#define PIN_ADC_CTRL_3 \
    {                  \
        DSY_GPIOA, 2   \
    }
#define PIN_ADC_CTRL_4 \
    {                  \
        DSY_GPIOA, 7   \
    }
#define PIN_ADC_CTRL_5 \
    {                  \
        DSY_GPIOB, 1   \
    }
#define PIN_ADC_CTRL_6 \
    {                  \
        DSY_GPIOC, 4   \
    }
#define PIN_ADC_CTRL_7 \
    {                  \
        DSY_GPIOC, 0   \
    }
#define PIN_ADC_CTRL_8 \
    {                  \
        DSY_GPIOC, 1   \
    }

#define DUMMYPIN     \
    {                \
        DSY_GPIOX, 0 \
    }

static const dsy_gpio_pin kPinMap[4][10] = {
    /** Header Bank A */
    {
        DUMMYPIN,        /**< A1  - -12V Power Input */
        {DSY_GPIOA, 1},  /**< A2  - UART1 Rx */
        {DSY_GPIOA, 0},  /**< A3  - UART1 Tx */
        DUMMYPIN,        /**< A4  - GND */
        DUMMYPIN,        /**< A5  - +12V Power Input */
        DUMMYPIN,        /**< A6  - +5V Power Output */
        DUMMYPIN,        /**< A7  - GND */
        {DSY_GPIOB, 14}, /**< A8  - USB DM */
        {DSY_GPIOB, 15}, /**< A9  - USB DP */
        DUMMYPIN,        /**< A10 - +3V3 Power Output */
    },
    /** Header Bank B */
    {
        DUMMYPIN,        /**< B1  - Audio Out Right */
        DUMMYPIN,        /**< B2  - Audio Out Left*/
        DUMMYPIN,        /**< B3  - Audio In Right */
        DUMMYPIN,        /**< B4  - Audio In Left */
        {DSY_GPIOC, 13}, /**< B5  - GATE OUT 1 */
        {DSY_GPIOC, 14}, /**< B6  - GATE OUT 2 */
        {DSY_GPIOB, 8},  /**< B7  - I2C1 SCL */
        {DSY_GPIOB, 9},  /**< B8  - I2C1 SDA */
        {DSY_GPIOG, 14}, /**< B9  - GATE IN 2 */
        {DSY_GPIOG, 13}, /**< B10 - GATE IN 1 */
    },
    /** Header Bank C */
    {
        {DSY_GPIOA, 5}, /**< C1  - CV Out 2 */
        PIN_ADC_CTRL_4, /**< C2  - CV In 4 */
        PIN_ADC_CTRL_3, /**< C3  - CV In 3 */
        PIN_ADC_CTRL_2, /**< C4  - CV In 2 */
        PIN_ADC_CTRL_1, /**< C5  - CV In 1 */
        PIN_ADC_CTRL_5, /**< C6  - CV In 5 */
        PIN_ADC_CTRL_6, /**< C7  - CV In 6 */
        PIN_ADC_CTRL_7, /**< C8  - CV In 7 */
        PIN_ADC_CTRL_8, /**< C9  - CV In 8 */
        {DSY_GPIOA, 4}, /**< C10 - CV Out 1 */
    },
    /** Header Bank D */
    {
        {DSY_GPIOB, 4},  /**< D1  - SPI2 CS */
        {DSY_GPIOC, 11}, /**< D2  - SDMMC D3 */
        {DSY_GPIOC, 10}, /**< D3  - SDMMC D2*/
        {DSY_GPIOC, 9},  /**< D4  - SDMMC D1*/
        {DSY_GPIOC, 8},  /**< D5  - SDMMC D0 */
        {DSY_GPIOC, 12}, /**< D6  - SDMMC CK */
        {DSY_GPIOD, 2},  /**< D7  - SDMMC CMD */
        {DSY_GPIOC, 2},  /**< D8  - SPI2 MISO */
        {DSY_GPIOC, 3},  /**< D9  - SPI2 MOSI */
        {DSY_GPIOD, 3},  /**< D10 - SPI2 SCK  */
    },
};

namespace daisy
{
void DaisyPatchSM::Init()
{
    /** Initialize the MCU and clock tree */
    System::Config syscfg;
    syscfg.Defaults();
    system.Init(syscfg);
    /** Memories */
    sdram.state                           = DSY_SDRAM_STATE_ENABLE;
    sdram.pin_config[DSY_SDRAM_PIN_SDNWE] = {DSY_GPIOH, 5};
    dsy_sdram_init(&sdram);
    qspi.pin_config[DSY_QSPI_PIN_IO0] = {DSY_GPIOF, 8};
    qspi.pin_config[DSY_QSPI_PIN_IO1] = {DSY_GPIOF, 9};
    qspi.pin_config[DSY_QSPI_PIN_IO2] = {DSY_GPIOF, 7};
    qspi.pin_config[DSY_QSPI_PIN_IO3] = {DSY_GPIOF, 6};
    qspi.pin_config[DSY_QSPI_PIN_CLK] = {DSY_GPIOF, 10};
    qspi.pin_config[DSY_QSPI_PIN_NCS] = {DSY_GPIOG, 6};
    qspi.device                       = DSY_QSPI_DEVICE_IS25LP064A;
    qspi.mode                         = DSY_QSPI_MODE_DSY_MEMORY_MAPPED;
    dsy_qspi_init(&qspi);
    dsy_qspi_init(&qspi);
    /** Audio */
    // Audio Init
    SaiHandle::Config sai_config;
    sai_config.periph          = SaiHandle::Config::Peripheral::SAI_1;
    sai_config.sr              = SaiHandle::Config::SampleRate::SAI_48KHZ;
    sai_config.bit_depth       = SaiHandle::Config::BitDepth::SAI_24BIT;
    sai_config.a_sync          = SaiHandle::Config::Sync::MASTER;
    sai_config.b_sync          = SaiHandle::Config::Sync::SLAVE;
    sai_config.a_dir           = SaiHandle::Config::Direction::RECEIVE;
    sai_config.b_dir           = SaiHandle::Config::Direction::TRANSMIT;
    sai_config.pin_config.fs   = {DSY_GPIOE, 4};
    sai_config.pin_config.mclk = {DSY_GPIOE, 2};
    sai_config.pin_config.sck  = {DSY_GPIOE, 5};
    sai_config.pin_config.sa   = {DSY_GPIOE, 6};
    sai_config.pin_config.sb   = {DSY_GPIOE, 3};
    SaiHandle sai_1_handle;
    sai_1_handle.Init(sai_config);
    I2CHandle::Config i2c_cfg;
    i2c_cfg.periph         = I2CHandle::Config::Peripheral::I2C_2;
    i2c_cfg.mode           = I2CHandle::Config::Mode::I2C_MASTER;
    i2c_cfg.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c_cfg.pin_config.scl = {DSY_GPIOB, 10};
    i2c_cfg.pin_config.sda = {DSY_GPIOB, 11};
    I2CHandle i2c2;
    i2c2.Init(i2c_cfg);
    codec.Init(i2c2);

    AudioHandle::Config audio_config;
    audio_config.blocksize  = 4;
    audio_config.samplerate = SaiHandle::Config::SampleRate::SAI_48KHZ;
    audio_config.postgain   = 1.f;
    audio.Init(audio_config, sai_1_handle);
    callback_rate_ = AudioSampleRate() / AudioBlockSize();

    /** ADC Init */
    AdcChannelConfig adc_config[kNumAdcInputs];
    dsy_gpio_pin     adc_pins[] = {
        PIN_ADC_CTRL_1,
        PIN_ADC_CTRL_2,
        PIN_ADC_CTRL_3,
        PIN_ADC_CTRL_4,
        PIN_ADC_CTRL_5,
        PIN_ADC_CTRL_6,
        PIN_ADC_CTRL_7,
        PIN_ADC_CTRL_8,
    };
    for(int i = 0; i < kNumAdcInputs; i++)
    {
        adc_config[i].InitSingle(adc_pins[i]);
    }
    adc.Init(adc_config, kNumAdcInputs);
    /** Control Init */
    for(size_t i = 0; i < kNumAdcInputs; i++)
    {
        controls[i].InitBipolarCv(adc.GetPtr(i), callback_rate_);
    }

    StartAdc();
}

void DaisyPatchSM::StartAudio(AudioHandle::AudioCallback cb)
{
    audio.Start(cb);
}

void DaisyPatchSM::StartAudio(AudioHandle::InterleavingAudioCallback cb)
{
    audio.Start(cb);
}

void DaisyPatchSM::ChangeAudioCallback(AudioHandle::AudioCallback cb)
{
    audio.ChangeCallback(cb);
}

void DaisyPatchSM::ChangeAudioCallback(
    AudioHandle::InterleavingAudioCallback cb)
{
    audio.ChangeCallback(cb);
}

void DaisyPatchSM::StopAudio()
{
    audio.Stop();
}

void DaisyPatchSM::SetAudioBlockSize(size_t size)
{
    audio.SetBlockSize(size);
    callback_rate_ = AudioSampleRate() / AudioBlockSize();
}

void DaisyPatchSM::SetAudioSampleRate(float sr)
{
    SaiHandle::Config::SampleRate sai_sr;
    switch(int(sr))
    {
        case 8000: sai_sr = SaiHandle::Config::SampleRate::SAI_8KHZ; break;
        case 16000: sai_sr = SaiHandle::Config::SampleRate::SAI_16KHZ; break;
        case 32000: sai_sr = SaiHandle::Config::SampleRate::SAI_32KHZ; break;
        case 48000: sai_sr = SaiHandle::Config::SampleRate::SAI_48KHZ; break;
        case 96000: sai_sr = SaiHandle::Config::SampleRate::SAI_96KHZ; break;
        default: sai_sr = SaiHandle::Config::SampleRate::SAI_48KHZ; break;
    }
    audio.SetSampleRate(sai_sr);
    callback_rate_ = AudioSampleRate() / AudioBlockSize();
}

size_t DaisyPatchSM::AudioBlockSize()
{
    return audio.GetConfig().blocksize;
}

float DaisyPatchSM::AudioSampleRate()
{
    return audio.GetSampleRate();
}

float DaisyPatchSM::AudioCallbackRate()
{
    return callback_rate_;
}

void DaisyPatchSM::StartAdc()
{
    adc.Start();
}

void DaisyPatchSM::StopAdc()
{
    adc.Stop();
}

void DaisyPatchSM::ProcessAnalogControls()
{
    for(int i = 0; i < kNumAdcInputs; i++)
    {
        controls[i].Process();
    }
}

void DaisyPatchSM::ProcessDigitalControls() {}

float DaisyPatchSM::GetAdcValue(int idx)
{
    return controls[idx].Value();
}

dsy_gpio_pin DaisyPatchSM::GetPin(PinBank bank, int idx)
{
    if(idx <= 0 || idx > 10)
        return DUMMYPIN;
    else
        return kPinMap[static_cast<int>(bank)][idx - 1];
}

bool DaisyPatchSM::ValidateSDRAM()
{
    uint32_t *sdramptr      = (uint32_t *)0xc0000000;
    uint32_t  size_in_words = 16777216;
    uint32_t  testval       = 0xdeadbeef;
    uint32_t  num_failed    = 0;
    /** Write test val */
    for(uint32_t i = 0; i < size_in_words; i++)
    {
        uint32_t *word = sdramptr + i;
        *word          = testval;
    }
    /** Compare written */
    for(uint32_t i = 0; i < size_in_words; i++)
    {
        uint32_t *word = sdramptr + i;
        if(*word != testval)
            num_failed++;
    }
    /** Write Zeroes */
    for(uint32_t i = 0; i < size_in_words; i++)
    {
        uint32_t *word = sdramptr + i;
        *word          = 0x00000000;
    }
    /** Compare Cleared */
    for(uint32_t i = 0; i < size_in_words; i++)
    {
        uint32_t *word = sdramptr + i;
        if(*word != 0)
            num_failed++;
    }
    return num_failed == 0;
}

} // namespace daisy