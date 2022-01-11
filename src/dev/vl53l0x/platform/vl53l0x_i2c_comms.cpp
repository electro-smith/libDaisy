#include "per/i2c.h"
#include "dev/vl53l0x/vl53l0x_def.h"
#include "dev/vl53l0x/vl53l0x_i2c_platform.h"

// int VL53L0X_i2c_init(I2CHandle *i2c) {
//   i2c->begin();
//   return VL53L0X_ERROR_NONE;
// }

namespace daisy
{
int VL53L0X_write_multi(uint8_t    deviceAddress,
                        uint8_t    index,
                        uint8_t *  pdata,
                        uint32_t   count,
                        I2CHandle *i2c)
{
    I2CHandle::Result res = i2c->WriteDataAtAddress(
        deviceAddress, index, 1, pdata, (uint16_t)count, 10);

    return res == I2CHandle::Result::OK ? 0 : 1;
}

int VL53L0X_read_multi(uint8_t    deviceAddress,
                       uint8_t    index,
                       uint8_t *  pdata,
                       uint32_t   count,
                       I2CHandle *i2c)
{
    I2CHandle::Result res = i2c->ReadDataAtAddress(
        deviceAddress, index, 1, pdata, (uint16_t)count, 10);
    return res == I2CHandle::Result::OK ? 0 : 1;
}

int VL53L0X_write_byte(uint8_t    deviceAddress,
                       uint8_t    index,
                       uint8_t    data,
                       I2CHandle *i2c)
{
    return VL53L0X_write_multi(deviceAddress, index, &data, 1, i2c);
}

int VL53L0X_write_word(uint8_t    deviceAddress,
                       uint8_t    index,
                       uint16_t   data,
                       I2CHandle *i2c)
{
    uint8_t buff[2];
    buff[1] = data & 0xFF;
    buff[0] = data >> 8;
    return VL53L0X_write_multi(deviceAddress, index, buff, 2, i2c);
}

int VL53L0X_write_dword(uint8_t    deviceAddress,
                        uint8_t    index,
                        uint32_t   data,
                        I2CHandle *i2c)
{
    uint8_t buff[4];

    buff[3] = data & 0xFF;
    buff[2] = data >> 8;
    buff[1] = data >> 16;
    buff[0] = data >> 24;

    return VL53L0X_write_multi(deviceAddress, index, buff, 4, i2c);
}

int VL53L0X_read_byte(uint8_t    deviceAddress,
                      uint8_t    index,
                      uint8_t *  data,
                      I2CHandle *i2c)
{
    return VL53L0X_read_multi(deviceAddress, index, data, 1, i2c);
}

int VL53L0X_read_word(uint8_t    deviceAddress,
                      uint8_t    index,
                      uint16_t * data,
                      I2CHandle *i2c)
{
    uint8_t buff[2];
    int     r = VL53L0X_read_multi(deviceAddress, index, buff, 2, i2c);

    uint16_t tmp;
    tmp = buff[0];
    tmp <<= 8;
    tmp |= buff[1];
    *data = tmp;

    return r;
}

int VL53L0X_read_dword(uint8_t    deviceAddress,
                       uint8_t    index,
                       uint32_t * data,
                       I2CHandle *i2c)
{
    uint8_t buff[4];
    int     r = VL53L0X_read_multi(deviceAddress, index, buff, 4, i2c);

    uint32_t tmp;
    tmp = buff[0];
    tmp <<= 8;
    tmp |= buff[1];
    tmp <<= 8;
    tmp |= buff[2];
    tmp <<= 8;
    tmp |= buff[3];

    *data = tmp;

    return r;
}

} // namespace daisy