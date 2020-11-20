#include "per/i2c.h"
#include "dev/codec_pcm3060.h"

// POWER-ON RESET and EXTERNAL RESET Sequence
// On DaisyPetalSM the RESET pin is held high on hardware,
// and is therefore not able to be reset from sw aside from
// with an I2C -> RESET message.

// Default Format spec for now: I2S 24-bit MSB aligned (FMT1/2[1:0] = 01)

// Slave address: 0b100011Nx
// N = 0 (GND on hardware)
// x = R/W|

// TODO:
// * add Deemphasis configuration
// * add filter-rolloff configuration
// * add format control configuration
// * add digital attenuation for adc/dac


const uint8_t kAddrRegSysCtrl       = 0x40;
const uint8_t kAddrRegDacAttenLeft  = 0x41;
const uint8_t kAddrRegDacAttenRight = 0x42;
const uint8_t kAddrRegDacCtrl1      = 0x43;
const uint8_t kAddrRegDacCtrl2      = 0x44;
const uint8_t kAddrRegDigCtrl       = 0x45;
const uint8_t kAddrRegAdcAttenLeft  = 0x46;
const uint8_t kAddrRegAdcAttenRight = 0x47;
const uint8_t kAddrRegAdcCtrl1      = 0x48;
const uint8_t kAddrRegAdcCtrl2      = 0x49;

// SysCtrl masks
// Reset masks
const uint8_t kMrstBitMask = 0x80;
const uint8_t kSrstBitMask = 0x40;

// DacCtrl1 Masks
// This can be used for FMT1[1:0] and FMT2[1:0]
const uint8_t kFmtBitMask = 0x03;


namespace daisy
{
void Pcm3060::Init(I2CHandle i2c)
{
    i2c_ = i2c;

    // bit 1 can be set via hardware and should be configurable.
    dev_addr_ = 0b10001100;

    // Reset the codec (though by default we may not need to do this)
    uint8_t dac_ctrl = ReadRegister(kAddrRegDacCtrl1);
    uint8_t adc_ctrl = ReadRegister(kAddrRegAdcCtrl1);
    dac_ctrl |= (kFmtBitMask & 1);
    adc_ctrl |= (kFmtBitMask & 1);
    WriteRegister(kAddrRegDacCtrl1, dac_ctrl);
    WriteRegister(kAddrRegAdcCtrl1, adc_ctrl);
}
uint8_t Pcm3060::ReadRegister(uint8_t addr)
{
    // Transmit devaddr with lsb as 0, a
    uint8_t data;
    i2c_.TransmitBlocking(dev_addr_, &addr, 1, 250);
    i2c_.ReceiveBlocking(dev_addr_, &data, 1, 250);
    return data;
}

void Pcm3060::WriteRegister(uint8_t addr, uint8_t val)
{
    uint8_t data[2];
    data[0] = addr;
    data[1] = val;
    i2c_.TransmitBlocking(dev_addr_ | (1 << 0), data, 2, 250);
}

} // namespace daisy
