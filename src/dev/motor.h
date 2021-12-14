/******************************************************************
 This is the library for the Adafruit Motor Shield V2 for Arduino (and Daisy!).
 It supports DC motors & Stepper motors with microstepping as well
 as stacking-support. It is *not* compatible with the V1 library!
 It will only work with https://www.adafruit.com/products/1483
 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source hardware
 by purchasing products from Adafruit!
 Written by Limor Fried/Ladyada for Adafruit Industries.
 Adapted by Corvus Prudens for the Daisy platform.
 BSD license, check license.txt for more information.
 All text above must be included in any redistribution.
 ******************************************************************/

#ifndef DSY_MOTOR_H
#define DSY_MOTOR_H

#include "pca9685.h"

#define MICROSTEPS 16 // 8 or 16

namespace daisy
{

/** Object that controls and keeps state for the whole motor shield.
    Use it to create DC and Stepper motor objects! */
class Adafruit_MotorShield
{
  public:
    Adafruit_MotorShield() {}
    ~Adafruit_MotorShield() {}

    struct Config
    {
        Config() { frequency = 1600; }

        Pca9685::Config pca9685_config;
        float           frequency;
    };

    enum Result
    {
        OK = 0,
        ERR
    };

    /** Object that controls and keeps state for a single DC motor */
    class Adafruit_DCMotor
    {
      public:
        Adafruit_DCMotor(void);
        friend class Adafruit_MotorShield; ///< Let MotorShield create DCMotors
        void Run(uint8_t);
        void SetSpeed(uint8_t);
        void SetSpeedFine(uint16_t speed);
        void FullOn();
        void FullOff();

      private:
        uint8_t PWMpin, IN1pin, IN2pin;
        uint8_t motornum;

        Adafruit_MotorShield *MC;
    };

    /** Object that controls and keeps state for a single stepper motor */
    class Adafruit_StepperMotor
    {
      public:
        Adafruit_StepperMotor(void);
        void SetSpeed(uint16_t);

        void    Step(uint16_t steps, uint8_t dir, uint8_t style = SINGLE);
        void    StepNonblocking(uint16_t steps, uint8_t dir, uint8_t style = SINGLE);
        uint8_t Onestep(uint8_t dir, uint8_t style);
        void    Release(void);
        void    Process(void);

        friend class
            Adafruit_MotorShield; ///< Let MotorShield create StepperMotors

      private:
        uint32_t usperstep;

        uint8_t  PWMApin, AIN1pin, AIN2pin;
        uint8_t  PWMBpin, BIN1pin, BIN2pin;
        uint16_t revsteps; // # steps per revolution
        uint8_t  currentstep;
        uint8_t  steppernum;

        uint8_t nonblock_dir, nonblock_style;
        uint16_t nonblock_steps;
        uint32_t nonblock_uspers, prev_micros;
        bool nonblock_active;

        Adafruit_MotorShield *MC;
    };

    Result                 Init(Config &config);
    Adafruit_DCMotor      *GetMotor(uint8_t n);
    Adafruit_StepperMotor *GetStepper(uint16_t steps, uint8_t n);

    void SetPWM(uint8_t pin, uint16_t val);
    void SetPin(uint8_t pin, bool val);

    static constexpr uint16_t DRIVER_INCREMENT = 16;

    static constexpr uint8_t MOTOR1_A = 2;
    static constexpr uint8_t MOTOR1_B = 3;
    static constexpr uint8_t MOTOR2_A = 1;
    static constexpr uint8_t MOTOR2_B = 4;
    static constexpr uint8_t MOTOR4_A = 0;
    static constexpr uint8_t MOTOR4_B = 6;
    static constexpr uint8_t MOTOR3_A = 5;
    static constexpr uint8_t MOTOR3_B = 7;

    static constexpr uint8_t FORWARD         = 1;
    static constexpr uint8_t BACKWARD        = 2;
    static constexpr uint8_t BRAKE           = 3;
    static constexpr uint8_t STEPPER_RELEASE = 4;

    static constexpr uint8_t SINGLE     = 1;
    static constexpr uint8_t DOUBLE     = 2;
    static constexpr uint8_t INTERLEAVE = 3;
    static constexpr uint8_t MICROSTEP  = 4;

  private:
    Adafruit_DCMotor      dcmotors[4];
    Adafruit_StepperMotor steppers[2];
    Pca9685               pwm_;
    Config                config_;
};

} // namespace daisy

#endif // DSY_MOTOR_H
