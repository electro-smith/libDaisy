#include "daisy_field.h"
#ifndef SAMPLE_RATE
#define SAMPLE_RATE DSY_AUDIO_SAMPLE_RATE /**< & */
#endif

//// Rev1 Pins
//// Hardware related defines.
//#define SW_1_PIN 29    /**< & */
//#define SW_2_PIN 28    /**< & */
//#define SW_3_PIN 27    /**< & */
//#define GATE_OUT_PIN 0 /**< & */
//#define GATE_IN_PIN 1  /**< & */
//
//// keyboard switches shift register
//#define KB_SW_SR_CS_PIN 8  /**< & */
//#define KB_SW_SR_CLK_PIN 9 /**< & */
//#define KB_SW_SR_D1_PIN 10 /**< & */
//#define KB_SW_SR_D2_PIN 11 /**< & */
//
//// UART for MIDI via TRS jacks on Field
//#define MIDI_OUT_PIN 14 /**< & */
//#define MIDI_IN_PIN 15  /**< & */
//
////CD4051 Select Pin controls
//#define MUX_SEL_0_PIN 21 /**< & */
//#define MUX_SEL_1_PIN 20 /**< & */
//#define MUX_SEL_2_PIN 19 /**< & */
//
//#define MUX_ADC_PIN 16 /**< & */
//#define CV1_ADC_PIN 17 /**< & */
//#define CV2_ADC_PIN 18 /**< & */
//#define CV3_ADC_PIN 23 /**< & */
//#define CV4_ADC_PIN 22 /**< & */

// Rev2 Pins
#define PIN_GATE_IN 0
#define PIN_SPI_CS 7
#define PIN_SPI_SCK 8
#define PIN_OLED_CMD 9
#define PIN_SPI_MOSI 10
#define PIN_I2C_SCL 11
#define PIN_I2C_SDA 12
#define PIN_MIDI_OUT 13
#define PIN_MIDI_IN 14
#define PIN_GATE_OUT 15
#define PIN_ADC_POT_MUX 16
#define PIN_ADC_CV_1 17
#define PIN_ADC_CV_2 18
#define PIN_MUX_SEL_2 19
#define PIN_MUX_SEL_1 20
#define PIN_MUX_SEL_0 21
#define PIN_CD4021_D1 26
#define PIN_CD4021_CS 27
#define PIN_CD4021_CLK 28
#define PIN_SW_2 29
#define PIN_SW_1 30

// IT LOOKS LIKE THESE MAY NEED TO GET SWAPPED.....
#define PIN_DAC_2 22    // Jumped on Rev2 from 24
#define PIN_DAC_1 23    // Jumped on Rev2 from 25
#define PIN_ADC_CV_4 24 // Jumped on Rev2 from 22
#define PIN_ADC_CV_3 25 // Jumped on Rev2 from 23

using namespace daisy;

static constexpr I2CHandle::Config field_led_i2c_config
    = {I2CHandle::Config::Peripheral::I2C_1,
       {{DSY_GPIOB, 8}, {DSY_GPIOB, 9}},
       I2CHandle::Config::Speed::I2C_1MHZ};

static LedDriverPca9685<2, true>::DmaBuffer DMA_BUFFER_MEM_SECTION
    field_led_dma_buffer_a,
    field_led_dma_buffer_b;

void DaisyField::Init()
{
    seed.Configure();
    seed.Init();
    samplerate_ = seed.AudioSampleRate();
    blocksize_  = 48;
    seed.SetAudioBlockSize(blocksize_);
    blockrate_ = samplerate_ / (float)blocksize_;

    // Switches
    uint8_t sw_pin[]  = {PIN_SW_1, PIN_SW_2};
    uint8_t adc_pin[] = {PIN_ADC_CV_1,
                         PIN_ADC_CV_2,
                         PIN_ADC_CV_3,
                         PIN_ADC_CV_4,
                         PIN_ADC_POT_MUX};

    for(size_t i = 0; i < SW_LAST; i++)
    {
        dsy_gpio_pin p = seed.GetPin(sw_pin[i]);
        sw_[i].Init(p, blockrate_);
    }

    // ADCs
    AdcChannelConfig adc_cfg[CV_LAST + 1];
    for(size_t i = 0; i < CV_LAST; i++)
    {
        adc_cfg[i].InitSingle(seed.GetPin(adc_pin[i]));
    }
    // POT MUX
    adc_cfg[CV_LAST].InitMux(seed.GetPin(PIN_ADC_POT_MUX),
                             8,
                             seed.GetPin(PIN_MUX_SEL_0),
                             seed.GetPin(PIN_MUX_SEL_1),
                             seed.GetPin(PIN_MUX_SEL_2));
    seed.adc.Init(adc_cfg, 5);

    for(size_t i = 0; i < KNOB_LAST; i++)
    {
        knob_[i].Init(seed.adc.GetMuxPtr(4, i), blockrate_);
    }
    for(size_t i = 0; i < CV_LAST; i++)
    {
        cv_[i].InitBipolarCv(seed.adc.GetPtr(i), blockrate_);
    }

    // Keyboard
    keyboard_sr_.pin_config[DSY_SR_4021_PIN_CS]   = seed.GetPin(PIN_CD4021_CS);
    keyboard_sr_.pin_config[DSY_SR_4021_PIN_CLK]  = seed.GetPin(PIN_CD4021_CLK);
    keyboard_sr_.pin_config[DSY_SR_4021_PIN_DATA] = seed.GetPin(PIN_CD4021_D1);
    keyboard_sr_.num_daisychained                 = 2;
    keyboard_sr_.num_parallel                     = 1;
    dsy_sr_4021_init(&keyboard_sr_);

    // OLED
    dsy_gpio_pin oled_pins[OledDisplay::NUM_PINS];
    oled_pins[OledDisplay::DATA_COMMAND] = seed.GetPin(PIN_OLED_CMD);
    oled_pins[OledDisplay::RESET]        = {DSY_GPIOX, 0}; // Not a real pin...
    display.Init(oled_pins);

    // LEDs
    // 2x PCA9685 addresses 0x00, and 0x02
    uint8_t   addr[2] = {0x00, 0x02};
    I2CHandle i2c;
    i2c.Init(field_led_i2c_config);
    led_driver_.Init(i2c, addr, field_led_dma_buffer_a, field_led_dma_buffer_b);

    // Gate In
    dsy_gpio_pin gate_in_pin;
    gate_in_pin = seed.GetPin(PIN_GATE_IN);
    gate_in_.Init(&gate_in_pin);
    // Gate Out
    gate_out_.mode = DSY_GPIO_MODE_OUTPUT_PP;
    gate_out_.pull = DSY_GPIO_NOPULL;
    gate_out_.pin  = seed.GetPin(PIN_GATE_OUT);
    dsy_gpio_init(&gate_out_);
    dsy_dac_init(&seed.dac_handle, DSY_DAC_CHN_BOTH);
    dsy_tim_start();
}

void DaisyField::VegasMode()
{
    uint32_t now;
    now = dsy_system_getnow();
    size_t idx;
    float  key_bright;
    // Cycle all 16 LEDs on keyboard SM in opposite pattern or something
    uint8_t led_grp_a[] = {LED_KEY_A8,
                           LED_KEY_A7,
                           LED_KEY_A6,
                           LED_KEY_A5,
                           LED_KEY_A4,
                           LED_KEY_A3,
                           LED_KEY_A2,
                           LED_KEY_A1};
    uint8_t led_grp_b[] = {LED_KEY_B8,
                           LED_KEY_B7,
                           LED_KEY_B6,
                           LED_KEY_B5,
                           LED_KEY_B4,
                           LED_KEY_B3,
                           LED_KEY_B2,
                           LED_KEY_B1};
    uint8_t led_grp_c[] = {LED_KNOB_1,
                           LED_KNOB_2,
                           LED_KNOB_3,
                           LED_KNOB_4,
                           LED_KNOB_5,
                           LED_KNOB_6,
                           LED_KNOB_7,
                           LED_KNOB_8};
    if(now - last_led_update_ > 10)
    {
        idx        = (now >> 10) % 8;
        key_bright = (float)(now & 1023) / 1023.0f;
        // Clear
        for(size_t i = 0; i < LED_LAST; i++)
        {
            led_driver_.SetLed(i, 0.0f);
        }
        // Knob LEDs dance in order
        led_driver_.SetLed(led_grp_a[idx], key_bright);
        led_driver_.SetLed(led_grp_b[idx], 1.0f - key_bright);
        led_driver_.SetLed(led_grp_c[idx], key_bright);
        // OLED moves a bar across the screen
        uint32_t bar_x = (now >> 4) % SSD1309_WIDTH;
        display.Fill(false);
        for(size_t i = 0; i < SSD1309_HEIGHT; i++)
        {
            display.DrawPixel(bar_x, i, true);
        }

        display.Update();
        led_driver_.SwapBuffersAndTransmit();
    }
}
