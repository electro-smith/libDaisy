/***************************************************
  This is a library for our Adafruit 16-channel PWM & Servo driver
  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/815
  These displays use I2C to communicate, 2 pins are required to
  interface. For Arduino UNOs, thats SCL -> Analog 5, SDA -> Analog 4
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "pca9685.h"
#include "system.h"

using namespace daisy;

Pca9685::Result Pca9685::Init(Pca9685::Config config)
{
    config_ = config;
    I2CHandle::Config i2c_config;
    i2c_config.mode   = I2CHandle::Config::Mode::I2C_MASTER;
    i2c_config.periph = config.periph;
    i2c_config.speed  = config.speed;

    i2c_config.pin_config.scl = config.scl;
    i2c_config.pin_config.sda = config.sda;
    return (Result)i2c_.Init(i2c_config);
}

void Pca9685::Reset()
{
    Write8(PCA9685_MODE1, 0x0);
}

void Pca9685::SetPWMFreq(float freq)
{
    freq *= 0.9; // Correct for overshoot in the frequency setting.

    float prescaleval = 25000000;
    prescaleval /= 4096;
    prescaleval /= freq;
    prescaleval -= 1;
    uint8_t prescale = (uint8_t)prescaleval + 0.5;

    uint8_t oldmode = Read8(PCA9685_MODE1);
    uint8_t newmode = (oldmode & 0x7F) | 0x10; // sleep
    Write8(PCA9685_MODE1, newmode);            // go to sleep
    Write8(PCA9685_PRESCALE, prescale);        // set the prescaler
    Write8(PCA9685_MODE1, oldmode);
    System::Delay(5);
    Write8(
        PCA9685_MODE1,
        oldmode
            | 0xa1); //  This sets the MODE1 register to turn on auto increment.
}

void Pca9685::SetPWM(uint8_t num, uint16_t on, uint16_t off)
{
    uint8_t buffer[5];
    buffer[0] = LED0_ON_L + 4 * num;
    buffer[1] = on;
    buffer[2] = on >> 8;
    buffer[3] = off;
    buffer[4] = off >> 8;
    i2c_.TransmitBlocking(config_.address, buffer, 5, 1);
}

uint8_t Pca9685::Read8(uint8_t addr)
{
    i2c_.TransmitBlocking(config_.address, &addr, 1, 1);
    i2c_.ReceiveBlocking(config_.address, &addr, 1, 1);
    return addr;
}

void Pca9685::Write8(uint8_t addr, uint8_t d)
{
    uint8_t buffer[2] = {addr, d};
    i2c_.TransmitBlocking(config_.address, buffer, 2, 1);
}
