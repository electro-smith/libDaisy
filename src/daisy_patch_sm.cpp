#include "daisy_patch_sm.h"
#include <vector>

namespace daisy
{
namespace patch_sm
{
    /** Const definitions */
    static constexpr Pin DUMMYPIN        = Pin(); //Pin(PORTX, 0);
    static constexpr Pin PIN_ADC_CTRL_1  = Pin(PORTA, 3);
    static constexpr Pin PIN_ADC_CTRL_2  = Pin(PORTA, 6);
    static constexpr Pin PIN_ADC_CTRL_3  = Pin(PORTA, 2);
    static constexpr Pin PIN_ADC_CTRL_4  = Pin(PORTA, 7);
    static constexpr Pin PIN_ADC_CTRL_5  = Pin(PORTB, 1);
    static constexpr Pin PIN_ADC_CTRL_6  = Pin(PORTC, 4);
    static constexpr Pin PIN_ADC_CTRL_7  = Pin(PORTC, 0);
    static constexpr Pin PIN_ADC_CTRL_8  = Pin(PORTC, 1);
    static constexpr Pin PIN_ADC_CTRL_9  = Pin(PORTA, 1);
    static constexpr Pin PIN_ADC_CTRL_10 = Pin(PORTA, 0);
    static constexpr Pin PIN_ADC_CTRL_11 = Pin(PORTC, 3);
    static constexpr Pin PIN_ADC_CTRL_12 = Pin(PORTC, 2);
    static constexpr Pin PIN_USER_LED    = Pin(PORTC, 7);

    const Pin kPinMap[4][10] = {
        /** Header Bank A */
        {
            DUMMYPIN,       /**< A1  - -12V Power Input */
            Pin(PORTA, 10), /**< A2  - UART1 Rx */
            Pin(PORTA, 0),  /**< A3  - UART1 Tx */
            DUMMYPIN,       /**< A4  - GND */
            DUMMYPIN,       /**< A5  - +12V Power Input */
            DUMMYPIN,       /**< A6  - +5V Power Output */
            DUMMYPIN,       /**< A7  - GND */
            Pin(PORTB, 14), /**< A8  - USB DM */
            Pin(PORTB, 15), /**< A9  - USB DP */
            DUMMYPIN,       /**< A10 - +3V3 Power Output */
        },
        /** Header Bank B */
        {
            DUMMYPIN,       /**< B1  - Audio Out Right */
            DUMMYPIN,       /**< B2  - Audio Out Left*/
            DUMMYPIN,       /**< B3  - Audio In Right */
            DUMMYPIN,       /**< B4  - Audio In Left */
            Pin(PORTC, 14), /**< B5  - GATE OUT 1 */
            Pin(PORTC, 13), /**< B6  - GATE OUT 2 */
            Pin(PORTB, 8),  /**< B7  - I2C1 SCL */
            Pin(PORTB, 9),  /**< B8  - I2C1 SDA */
            Pin(PORTG, 14), /**< B9  - GATE IN 2 */
            Pin(PORTG, 13), /**< B10 - GATE IN 1 */
        },
        /** Header Bank C */
        {
            Pin(PORTA, 5),  /**< C1  - CV Out 2 */
            PIN_ADC_CTRL_4, /**< C2  - CV In 4 */
            PIN_ADC_CTRL_3, /**< C3  - CV In 3 */
            PIN_ADC_CTRL_2, /**< C4  - CV In 2 */
            PIN_ADC_CTRL_1, /**< C5  - CV In 1 */
            PIN_ADC_CTRL_5, /**< C6  - CV In 5 */
            PIN_ADC_CTRL_6, /**< C7  - CV In 6 */
            PIN_ADC_CTRL_7, /**< C8  - CV In 7 */
            PIN_ADC_CTRL_8, /**< C9  - CV In 8 */
            Pin(PORTA, 4),  /**< C10 - CV Out 1 */
        },
        /** Header Bank D */
        {
            Pin(PORTB, 4),  /**< D1  - SPI2 CS */
            Pin(PORTC, 11), /**< D2  - SDMMC D3 */
            Pin(PORTC, 10), /**< D3  - SDMMC D2*/
            Pin(PORTC, 9),  /**< D4  - SDMMC D1*/
            Pin(PORTC, 8),  /**< D5  - SDMMC D0 */
            Pin(PORTC, 12), /**< D6  - SDMMC CK */
            Pin(PORTD, 2),  /**< D7  - SDMMC CMD */
            Pin(PORTC, 2),  /**< D8  - SPI2 MISO */
            Pin(PORTC, 3),  /**< D9  - SPI2 MOSI */
            Pin(PORTD, 3),  /**< D10 - SPI2 SCK  */
        },
    };

    const Pin DaisyPatchSM::A1  = kPinMap[0][0];
    const Pin DaisyPatchSM::A2  = kPinMap[0][1];
    const Pin DaisyPatchSM::A3  = kPinMap[0][2];
    const Pin DaisyPatchSM::A4  = kPinMap[0][3];
    const Pin DaisyPatchSM::A5  = kPinMap[0][4];
    const Pin DaisyPatchSM::A6  = kPinMap[0][5];
    const Pin DaisyPatchSM::A7  = kPinMap[0][6];
    const Pin DaisyPatchSM::A8  = kPinMap[0][7];
    const Pin DaisyPatchSM::A9  = kPinMap[0][8];
    const Pin DaisyPatchSM::A10 = kPinMap[0][9];
    const Pin DaisyPatchSM::B1  = kPinMap[1][0];
    const Pin DaisyPatchSM::B2  = kPinMap[1][1];
    const Pin DaisyPatchSM::B3  = kPinMap[1][2];
    const Pin DaisyPatchSM::B4  = kPinMap[1][3];
    const Pin DaisyPatchSM::B5  = kPinMap[1][4];
    const Pin DaisyPatchSM::B6  = kPinMap[1][5];
    const Pin DaisyPatchSM::B7  = kPinMap[1][6];
    const Pin DaisyPatchSM::B8  = kPinMap[1][7];
    const Pin DaisyPatchSM::B9  = kPinMap[1][8];
    const Pin DaisyPatchSM::B10 = kPinMap[1][9];
    const Pin DaisyPatchSM::C1  = kPinMap[2][0];
    const Pin DaisyPatchSM::C2  = kPinMap[2][1];
    const Pin DaisyPatchSM::C3  = kPinMap[2][2];
    const Pin DaisyPatchSM::C4  = kPinMap[2][3];
    const Pin DaisyPatchSM::C5  = kPinMap[2][4];
    const Pin DaisyPatchSM::C6  = kPinMap[2][5];
    const Pin DaisyPatchSM::C7  = kPinMap[2][6];
    const Pin DaisyPatchSM::C8  = kPinMap[2][7];
    const Pin DaisyPatchSM::C9  = kPinMap[2][8];
    const Pin DaisyPatchSM::C10 = kPinMap[2][9];
    const Pin DaisyPatchSM::D1  = kPinMap[3][0];
    const Pin DaisyPatchSM::D2  = kPinMap[3][1];
    const Pin DaisyPatchSM::D3  = kPinMap[3][2];
    const Pin DaisyPatchSM::D4  = kPinMap[3][3];
    const Pin DaisyPatchSM::D5  = kPinMap[3][4];
    const Pin DaisyPatchSM::D6  = kPinMap[3][5];
    const Pin DaisyPatchSM::D7  = kPinMap[3][6];
    const Pin DaisyPatchSM::D8  = kPinMap[3][7];
    const Pin DaisyPatchSM::D9  = kPinMap[3][8];
    const Pin DaisyPatchSM::D10 = kPinMap[3][9];

    /** outside of class static buffer(s) for DMA access */
    uint16_t DMA_BUFFER_MEM_SECTION dsy_patch_sm_dac_buffer[2][48];

    class DaisyPatchSM::Impl
    {
      public:
        Impl()
        {
            dac_running_            = false;
            dac_buffer_size_        = 48;
            dac_output_[0]          = 0;
            dac_output_[1]          = 0;
            internal_dac_buffer_[0] = dsy_patch_sm_dac_buffer[0];
            internal_dac_buffer_[1] = dsy_patch_sm_dac_buffer[1];
        }

        void InitDac();

        void StartDac(DacHandle::DacCallback callback);

        void StopDac();

        static void InternalDacCallback(uint16_t **output, size_t size);

        /** Based on a 0-5V output with a 0-4095 12-bit DAC */
        static inline uint16_t VoltageToCode(float input)
        {
            float pre = input * 819.f;
            if(pre > 4095.f)
                pre = 4095.f;
            else if(pre < 0.f)
                pre = 0.f;
            return (uint16_t)pre;
        }

        inline void WriteCvOut(int channel, float voltage)
        {
            if(channel == 0 || channel == 1)
                dac_output_[0] = VoltageToCode(voltage);
            if(channel == 0 || channel == 2)
                dac_output_[1] = VoltageToCode(voltage);
        }

        size_t    dac_buffer_size_;
        uint16_t *internal_dac_buffer_[2];
        uint16_t  dac_output_[2];
        DacHandle dac_;

      private:
        bool dac_running_;
    };

    /** Static Local Object */
    static DaisyPatchSM::Impl patch_sm_hw;

    /** Impl function definintions */

    void DaisyPatchSM::Impl::InitDac()
    {
        DacHandle::Config dac_config;
        dac_config.mode     = DacHandle::Mode::DMA;
        dac_config.bitdepth = DacHandle::BitDepth::
            BITS_12; /**< Sets the output value to 0-4095 */
        dac_config.chn               = DacHandle::Channel::BOTH;
        dac_config.buff_state        = DacHandle::BufferState::ENABLED;
        dac_config.target_samplerate = 48000;
        dac_.Init(dac_config);
    }

    void DaisyPatchSM::Impl::StartDac(DacHandle::DacCallback callback)
    {
        if(dac_running_)
            dac_.Stop();
        dac_.Start(internal_dac_buffer_[0],
                   internal_dac_buffer_[1],
                   dac_buffer_size_,
                   callback == nullptr ? InternalDacCallback : callback);
        dac_running_ = true;
    }

    void DaisyPatchSM::Impl::StopDac()
    {
        dac_.Stop();
        dac_running_ = false;
    }


    void DaisyPatchSM::Impl::InternalDacCallback(uint16_t **output, size_t size)
    {
        /** We could add some smoothing, interp, or something to make this a bit less waste-y */
        // std::fill(&output[0][0], &output[0][size], patch_sm_hw.dac_output_[0]);
        // std::fill(&output[1][1], &output[1][size], patch_sm_hw.dac_output_[1]);
        for(size_t i = 0; i < size; i++)
        {
            output[0][i] = patch_sm_hw.dac_output_[0];
            output[1][i] = patch_sm_hw.dac_output_[1];
        }
    }

    /** Actual DaisyPatchSM implementation
 *  With the pimpl model in place, we can/should probably
 *  move the rest of the implementation to the Impl class
 */

    void DaisyPatchSM::Init()
    {
        /** Assign pimpl pointer */
        pimpl_ = &patch_sm_hw;
        /** Initialize the MCU and clock tree */
        System::Config syscfg;
        syscfg.Boost();

        auto memory = System::GetProgramMemoryRegion();
        if(memory != System::MemoryRegion::INTERNAL_FLASH)
            syscfg.skip_clocks = true;

        system.Init(syscfg);
        /** Memories */
        if(memory == System::MemoryRegion::INTERNAL_FLASH)
        {
            /** FMC SDRAM */
            sdram.Init();
        }
        if(memory != System::MemoryRegion::QSPI)
        {
            /** QUADSPI FLASH */
            QSPIHandle::Config qspi_config;
            qspi_config.device = QSPIHandle::Config::Device::IS25LP064A;
            qspi_config.mode   = QSPIHandle::Config::Mode::MEMORY_MAPPED;
            qspi_config.pin_config.io0 = Pin(PORTF, 8);
            qspi_config.pin_config.io1 = Pin(PORTF, 9);
            qspi_config.pin_config.io2 = Pin(PORTF, 7);
            qspi_config.pin_config.io3 = Pin(PORTF, 6);
            qspi_config.pin_config.clk = Pin(PORTF, 10);
            qspi_config.pin_config.ncs = Pin(PORTG, 6);
            qspi.Init(qspi_config);
        }
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
        sai_config.pin_config.fs   = Pin(PORTE, 4);
        sai_config.pin_config.mclk = Pin(PORTE, 2);
        sai_config.pin_config.sck  = Pin(PORTE, 5);
        sai_config.pin_config.sa   = Pin(PORTE, 6);
        sai_config.pin_config.sb   = Pin(PORTE, 3);
        SaiHandle sai_1_handle;
        sai_1_handle.Init(sai_config);
        I2CHandle::Config i2c_cfg;
        i2c_cfg.periph         = I2CHandle::Config::Peripheral::I2C_2;
        i2c_cfg.mode           = I2CHandle::Config::Mode::I2C_MASTER;
        i2c_cfg.speed          = I2CHandle::Config::Speed::I2C_400KHZ;
        i2c_cfg.pin_config.scl = Pin(PORTB, 10);
        i2c_cfg.pin_config.sda = Pin(PORTB, 11);
        I2CHandle i2c2;
        i2c2.Init(i2c_cfg);
        codec.Init(i2c2);

        AudioHandle::Config audio_config;
        audio_config.blocksize  = 48;
        audio_config.samplerate = SaiHandle::Config::SampleRate::SAI_48KHZ;
        audio_config.postgain   = 1.f;
        audio.Init(audio_config, sai_1_handle);
        callback_rate_ = AudioSampleRate() / AudioBlockSize();

        /** ADC Init */
        AdcChannelConfig adc_config[ADC_LAST];
        /** Order of pins to match enum expectations */
        dsy_gpio_pin adc_pins[] = {
            PIN_ADC_CTRL_1,
            PIN_ADC_CTRL_2,
            PIN_ADC_CTRL_3,
            PIN_ADC_CTRL_4,
            PIN_ADC_CTRL_8,
            PIN_ADC_CTRL_7,
            PIN_ADC_CTRL_5,
            PIN_ADC_CTRL_6,
            PIN_ADC_CTRL_9,
            PIN_ADC_CTRL_10,
            PIN_ADC_CTRL_11,
            PIN_ADC_CTRL_12,
        };

        for(int i = 0; i < ADC_LAST; i++)
        {
            adc_config[i].InitSingle(adc_pins[i]);
        }
        adc.Init(adc_config, ADC_LAST);
        /** Control Init */
        for(size_t i = 0; i < ADC_LAST; i++)
        {
            if(i < ADC_9)
                controls[i].InitBipolarCv(adc.GetPtr(i), callback_rate_);
            else
                controls[i].Init(adc.GetPtr(i), callback_rate_);
        }

        /** Fixed-function Digital I/O */
        user_led.mode = DSY_GPIO_MODE_OUTPUT_PP;
        user_led.pull = DSY_GPIO_NOPULL;
        user_led.pin  = PIN_USER_LED;
        dsy_gpio_init(&user_led);
        //gate_in_1.Init((dsy_gpio_pin *)&DaisyPatchSM::B10);
        gate_in_1.Init((dsy_gpio_pin *)&B10);
        gate_in_2.Init((dsy_gpio_pin *)&B9);

        gate_out_1.mode = DSY_GPIO_MODE_OUTPUT_PP;
        gate_out_1.pull = DSY_GPIO_NOPULL;
        gate_out_1.pin  = B5;
        dsy_gpio_init(&gate_out_1);

        gate_out_2.mode = DSY_GPIO_MODE_OUTPUT_PP;
        gate_out_2.pull = DSY_GPIO_NOPULL;
        gate_out_2.pin  = B6;
        dsy_gpio_init(&gate_out_2);

        /** DAC init */
        pimpl_->InitDac();

        /** Start any background stuff */
        StartAdc();
        StartDac();
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

    void
    DaisyPatchSM::ChangeAudioCallback(AudioHandle::InterleavingAudioCallback cb)
    {
        audio.ChangeCallback(cb);
    }

    void DaisyPatchSM::StopAudio() { audio.Stop(); }

    void DaisyPatchSM::SetAudioBlockSize(size_t size)
    {
        audio.SetBlockSize(size);
        callback_rate_ = AudioSampleRate() / AudioBlockSize();
        for(size_t i = 0; i < ADC_LAST; i++)
        {
            controls[i].SetSampleRate(callback_rate_);
        }
    }

    void DaisyPatchSM::SetAudioSampleRate(float sr)
    {
        SaiHandle::Config::SampleRate sai_sr;
        switch(int(sr))
        {
            case 8000: sai_sr = SaiHandle::Config::SampleRate::SAI_8KHZ; break;
            case 16000:
                sai_sr = SaiHandle::Config::SampleRate::SAI_16KHZ;
                break;
            case 32000:
                sai_sr = SaiHandle::Config::SampleRate::SAI_32KHZ;
                break;
            case 48000:
                sai_sr = SaiHandle::Config::SampleRate::SAI_48KHZ;
                break;
            case 96000:
                sai_sr = SaiHandle::Config::SampleRate::SAI_96KHZ;
                break;
            default: sai_sr = SaiHandle::Config::SampleRate::SAI_48KHZ; break;
        }
        audio.SetSampleRate(sai_sr);
        callback_rate_ = AudioSampleRate() / AudioBlockSize();
        for(size_t i = 0; i < ADC_LAST; i++)
        {
            controls[i].SetSampleRate(callback_rate_);
        }
    }

    void
    DaisyPatchSM::SetAudioSampleRate(SaiHandle::Config::SampleRate sample_rate)
    {
        audio.SetSampleRate(sample_rate);
        callback_rate_ = AudioSampleRate() / AudioBlockSize();
        for(size_t i = 0; i < ADC_LAST; i++)
        {
            controls[i].SetSampleRate(callback_rate_);
        }
    }

    size_t DaisyPatchSM::AudioBlockSize()
    {
        return audio.GetConfig().blocksize;
    }

    float DaisyPatchSM::AudioSampleRate() { return audio.GetSampleRate(); }

    float DaisyPatchSM::AudioCallbackRate() { return callback_rate_; }

    void DaisyPatchSM::StartAdc() { adc.Start(); }

    void DaisyPatchSM::StopAdc() { adc.Stop(); }

    void DaisyPatchSM::ProcessAnalogControls()
    {
        for(int i = 0; i < ADC_LAST; i++)
        {
            controls[i].Process();
        }
    }

    void DaisyPatchSM::ProcessDigitalControls() {}

    float DaisyPatchSM::GetAdcValue(int idx) { return controls[idx].Value(); }

    Pin DaisyPatchSM::GetPin(const PinBank bank, const int idx)
    {
        if(idx <= 0 || idx > 10)
            return DUMMYPIN;
        else
            return kPinMap[static_cast<int>(bank)][idx - 1];
    }

    void DaisyPatchSM::StartDac(DacHandle::DacCallback callback)
    {
        pimpl_->StartDac(callback);
    }

    void DaisyPatchSM::StopDac() { pimpl_->StopDac(); }

    void DaisyPatchSM::WriteCvOut(const int channel, float voltage)
    {
        pimpl_->WriteCvOut(channel, voltage);
    }

    void DaisyPatchSM::SetLed(bool state) { dsy_gpio_write(&user_led, state); }

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

    bool DaisyPatchSM::ValidateQSPI(bool quick)
    {
        uint32_t start;
        uint32_t size;
        if(quick)
        {
            start = 0x400000;
            size  = 0x4000;
        }
        else
        {
            start = 0;
            size  = 0x800000;
        }
        // Erase the section to be tested
        qspi.Erase(start, start + size);
        // Create some test data
        std::vector<uint8_t> test;
        test.resize(size);
        uint8_t *testmem = test.data();
        for(size_t i = 0; i < size; i++)
            testmem[i] = (uint8_t)(i & 0xff);
        // Write the test data to the device
        qspi.Write(start, size, testmem);
        // Read it all back and count any/all errors
        // I supppose any byte where ((data & 0xff) == data)
        // would be able to false-pass..
        size_t fail_cnt = 0;
        for(size_t i = 0; i < size; i++)
            if(testmem[i] != (uint8_t)(i & 0xff))
                fail_cnt++;
        return fail_cnt == 0;
    }

} // namespace patch_sm

} // namespace daisy