#include "daisy_patch_sm.h"
#include <vector>

namespace daisy
{
namespace patch_sm
{
    /** Const definitions */
    constexpr Pin DUMMYPIN        = Pin(PORTX, 0);
    constexpr Pin PIN_ADC_CTRL_1  = DaisyPatchSM::C5;
    constexpr Pin PIN_ADC_CTRL_2  = DaisyPatchSM::C4;
    constexpr Pin PIN_ADC_CTRL_3  = DaisyPatchSM::C3;
    constexpr Pin PIN_ADC_CTRL_4  = DaisyPatchSM::C2;
    constexpr Pin PIN_ADC_CTRL_5  = DaisyPatchSM::C6;
    constexpr Pin PIN_ADC_CTRL_6  = DaisyPatchSM::C7;
    constexpr Pin PIN_ADC_CTRL_7  = DaisyPatchSM::C8;
    constexpr Pin PIN_ADC_CTRL_8  = DaisyPatchSM::C9;
    constexpr Pin PIN_ADC_CTRL_9  = DaisyPatchSM::A2;
    constexpr Pin PIN_ADC_CTRL_10 = DaisyPatchSM::A3;
    constexpr Pin PIN_ADC_CTRL_11 = DaisyPatchSM::D9;
    constexpr Pin PIN_ADC_CTRL_12 = DaisyPatchSM::D8;
    constexpr Pin PIN_USER_LED    = Pin(PORTC, 7);

    /** @note This is an adapter for the new Pin mapping system in the class so that
     *  GetPin still works. If GetPin is removed (i.e. the next major version), this should also be removed 
    */
    constexpr Pin kPinMap[4][10] =
        // Bank A
        {{DaisyPatchSM::A1,
          DaisyPatchSM::A2,
          DaisyPatchSM::A3,
          DaisyPatchSM::A4,
          DaisyPatchSM::A5,
          DaisyPatchSM::A6,
          DaisyPatchSM::A7,
          DaisyPatchSM::A8,
          DaisyPatchSM::A9,
          DaisyPatchSM::A10},

         // Bank B
         {DaisyPatchSM::B1,
          DaisyPatchSM::B2,
          DaisyPatchSM::B3,
          DaisyPatchSM::B4,
          DaisyPatchSM::B5,
          DaisyPatchSM::B6,
          DaisyPatchSM::B7,
          DaisyPatchSM::B8,
          DaisyPatchSM::B9,
          DaisyPatchSM::B10},

         // Bank C
         {DaisyPatchSM::C1,
          DaisyPatchSM::C2,
          DaisyPatchSM::C3,
          DaisyPatchSM::C4,
          DaisyPatchSM::C5,
          DaisyPatchSM::C6,
          DaisyPatchSM::C7,
          DaisyPatchSM::C8,
          DaisyPatchSM::C9,
          DaisyPatchSM::C10},

         // Bank D
         {DaisyPatchSM::D1,
          DaisyPatchSM::D2,
          DaisyPatchSM::D3,
          DaisyPatchSM::D4,
          DaisyPatchSM::D5,
          DaisyPatchSM::D6,
          DaisyPatchSM::D7,
          DaisyPatchSM::D8,
          DaisyPatchSM::D9,
          DaisyPatchSM::D10}};

    constexpr Pin DaisyPatchSM::A1;
    constexpr Pin DaisyPatchSM::A2;
    constexpr Pin DaisyPatchSM::A3;
    constexpr Pin DaisyPatchSM::A4;
    constexpr Pin DaisyPatchSM::A5;
    constexpr Pin DaisyPatchSM::A6;
    constexpr Pin DaisyPatchSM::A7;
    constexpr Pin DaisyPatchSM::A8;
    constexpr Pin DaisyPatchSM::A9;
    constexpr Pin DaisyPatchSM::A10;

    constexpr Pin DaisyPatchSM::B1;
    constexpr Pin DaisyPatchSM::B2;
    constexpr Pin DaisyPatchSM::B3;
    constexpr Pin DaisyPatchSM::B4;
    constexpr Pin DaisyPatchSM::B5;
    constexpr Pin DaisyPatchSM::B6;
    constexpr Pin DaisyPatchSM::B7;
    constexpr Pin DaisyPatchSM::B8;
    constexpr Pin DaisyPatchSM::B9;
    constexpr Pin DaisyPatchSM::B10;

    constexpr Pin DaisyPatchSM::C1;
    constexpr Pin DaisyPatchSM::C2;
    constexpr Pin DaisyPatchSM::C3;
    constexpr Pin DaisyPatchSM::C4;
    constexpr Pin DaisyPatchSM::C5;
    constexpr Pin DaisyPatchSM::C6;
    constexpr Pin DaisyPatchSM::C7;
    constexpr Pin DaisyPatchSM::C8;
    constexpr Pin DaisyPatchSM::C9;
    constexpr Pin DaisyPatchSM::C10;

    constexpr Pin DaisyPatchSM::D1;
    constexpr Pin DaisyPatchSM::D2;
    constexpr Pin DaisyPatchSM::D3;
    constexpr Pin DaisyPatchSM::D4;
    constexpr Pin DaisyPatchSM::D5;
    constexpr Pin DaisyPatchSM::D6;
    constexpr Pin DaisyPatchSM::D7;
    constexpr Pin DaisyPatchSM::D8;
    constexpr Pin DaisyPatchSM::D9;
    constexpr Pin DaisyPatchSM::D10;

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

        auto memory       = System::GetProgramMemoryRegion();
        auto boot_version = System::GetBootloaderVersion();

        // When using the bootloader prior to v6, clocks have been already configured
        if(boot_version == System::BootInfo::Version::LT_v6_0
           && memory != System::MemoryRegion::INTERNAL_FLASH)
        {
            syscfg.skip_clocks = true;
        }

        system.Init(syscfg);
        /** Memories */
        // When using the bootloader priori to v6, SDRAM has been already configured
        if(boot_version != System::BootInfo::Version::LT_v6_0
           || (boot_version == System::BootInfo::Version::LT_v6_0
               && memory == System::MemoryRegion::INTERNAL_FLASH))
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
        constexpr Pin adc_pins[] = {
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
        gate_in_1.Init(B10);
        gate_in_2.Init(B9);

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

    dsy_gpio_pin DaisyPatchSM::GetPin(const PinBank bank, const int idx)
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