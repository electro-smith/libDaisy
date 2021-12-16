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

#ifndef DSY_PCA9685_H
#define DSY_PCA9685_H

#include "per/i2c.h"

#define PCA9685_SUBADR1 0x2
#define PCA9685_SUBADR2 0x3
#define PCA9685_SUBADR3 0x4

// #define PCA9685_MODE1 0x0
// #define PCA9685_PRESCALE 0xFE

#define LED0_ON_L 0x6
#define LED0_ON_H 0x7
#define LED0_OFF_L 0x8
#define LED0_OFF_H 0x9

#define ALLLED_ON_L 0xFA
#define ALLLED_ON_H 0xFB
#define ALLLED_OFF_L 0xFC
#define ALLLED_OFF_H 0xFD

namespace daisy
{

class Pca9685
{
  public:
    struct Config
    {
        Config()
        {
            periph = I2CHandle::Config::Peripheral::I2C_1;
            speed  = I2CHandle::Config::Speed::I2C_400KHZ;

            scl = {DSY_GPIOB, 8};
            sda = {DSY_GPIOB, 9};

            address = PCA9685_I2C_BASE_ADDRESS;
        }

        uint8_t                       address;
        I2CHandle::Config::Peripheral periph;
        I2CHandle::Config::Speed      speed;
        dsy_gpio_pin                  scl;
        dsy_gpio_pin                  sda;
    };

    enum Result
    {
        OK = 0,
        ERR
    };

    Pca9685() {}
    ~Pca9685() {}

    Result Init(Config config);
    void   Reset(void);
    void   SetPWMFreq(float freq);
    void   SetPWM(uint8_t num, uint16_t on, uint16_t off);

    static constexpr uint8_t PCA9685_I2C_BASE_ADDRESS = 0b01000000;
    static constexpr uint8_t PCA9685_MODE1
        = 0x00; // location for Mode1 register address
    static constexpr uint8_t PCA9685_MODE2
        = 0x01; // location for Mode2 reigster address
    static constexpr uint8_t PCA9685_LED0
        = 0x06; // location for start of LED0 registers
    static constexpr uint8_t PCA9685_PRESCALE
        = 0xFE; //location for setting prescale (clock speed)

  private:
    Config    config_;
    I2CHandle i2c_;

    uint8_t Read8(uint8_t addr);
    void    Write8(uint8_t addr, uint8_t d);
};

} // namespace daisy

#endif // DSY_PCA9685_H
