/******************************************************************
 This is the library for the Adafruit Motor Shield V2 for Arduino.
 It supports DC motors & Stepper motors with microstepping as well
 as stacking-support. It is *not* compatible with the V1 library!
 It will only work with https://www.adafruit.com/products/1483
 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source hardware
 by purchasing products from Adafruit!
 Written by Limor Fried/Ladyada for Adafruit Industries.
 BSD license, check license.txt for more information.
 All text above must be included in any redistribution.
 ******************************************************************/

#ifndef DSY_MOTOR_H
#define DSY_MOTOR_H

#include "leddriver.h"

namespace daisy 
{

#define DRIVER_INCREMENT 16

#define MICROSTEPS 16 // 8 or 16

#define MOTOR1_A 2
#define MOTOR1_B 3
#define MOTOR2_A 1
#define MOTOR2_B 4
#define MOTOR4_A 0
#define MOTOR4_B 6
#define MOTOR3_A 5
#define MOTOR3_B 7

#define FORWARD 1
#define BACKWARD 2
#define BRAKE 3
#define RELEASE 4

#define SINGLE 1
#define DOUBLE 2
#define INTERLEAVE 3
#define MICROSTEP 4

#define HIGH 1
#define LOW 0

/** Object that controls and keeps state for the whole motor shield.
    Use it to create DC and Stepper motor objects! */
template<size_t numDrivers>
class Adafruit_MotorShield {
  public:

    typedef LedDriverPca9685<numDrivers>::DmaBuffer DmaBuffer;

    struct Config {
      I2CHandle::Config i2c_config; // Passed on the the PCA6985
      const uint8_t (&addresses)[numDrivers];
      DmaBuffer dma_buffer_a;
      DmaBuffer dma_buffer_b;
      dsy_gpio_pin oe_pin = {DSY_GPIOX, 0};
    };

    Adafruit_MotorShield();
    ~Adafruit_MotorShield();

    /** Object that controls and keeps state for a single DC motor */
    class Adafruit_DCMotor {
      public:
        Adafruit_DCMotor(void);
        friend class Adafruit_MotorShield; ///< Let MotorShield create DCMotors
        void run(uint8_t);
        void setSpeed(uint8_t);
        void setSpeedFine(uint16_t speed);
        void fullOn();
        void fullOff();

      private:
        uint8_t PWMpin, IN1pin, IN2pin;
        Adafruit_MotorShield *MC;
        uint8_t motornum;
    };

    /** Object that controls and keeps state for a single stepper motor */
    class Adafruit_StepperMotor {
      public:
        Adafruit_StepperMotor(void);
        void setSpeed(uint16_t);

        void step(uint16_t steps, uint8_t dir, uint8_t style = SINGLE);
        uint8_t onestep(uint8_t dir, uint8_t style);
        void release(void);

        friend class Adafruit_MotorShield; ///< Let MotorShield create StepperMotors

      private:
        uint32_t usperstep;

        uint8_t PWMApin, AIN1pin, AIN2pin;
        uint8_t PWMBpin, BIN1pin, BIN2pin;
        uint16_t revsteps; // # steps per revolution
        uint8_t currentstep;
        Adafruit_MotorShield *MC;
        uint8_t steppernum;
    };

    bool Init(Config config);
    Adafruit_DCMotor *getMotor(uint8_t n);
    Adafruit_StepperMotor *getStepper(uint16_t steps, uint8_t n);

    void setPWM(uint8_t pin, uint16_t val);
    void setPin(uint8_t pin, bool val);

  private:
    uint8_t _addr;
    uint16_t _freq;
    Adafruit_DCMotor dcmotors[4];
    Adafruit_StepperMotor steppers[2];
    daisy::LedDriverPca9685<1> pwm_;
  
};

/*!
 * @file Adafruit_MotorShield.cpp
 *
 * @mainpage Adafruit FXOS8700 accel/mag sensor driver
 *
 * @section intro_sec Introduction
 *
 * This is the library for the Adafruit Motor Shield V2 for Arduino.
 * It supports DC motors & Stepper motors with microstepping as well
 * as stacking-support. It is *not* compatible with the V1 library!
 * For use with the Motor Shield https://www.adafruit.com/products/1483
 * and Motor FeatherWing https://www.adafruit.com/product/2927
 *
 * This shield/wing uses I2C to communicate, 2 pins (SCL+SDA) are required
 * to interface.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section author Author
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries.
 *
 * @section license License
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

// #include "Adafruit_MotorShield.h"
// #include "Arduino.h"
// #include <Adafruit_MS_PWMServoDriver.h>

#if (MICROSTEPS == 8)
///! A sinusoial microstepping curve for the PWM output (8-bit range) with 9
/// points - last one is start of next step.
static uint8_t microstepcurve[] = {0, 50, 98, 142, 180, 212, 236, 250, 255};
#elif (MICROSTEPS == 16)
///! A sinusoial microstepping curve for the PWM output (8-bit range) with 17
/// points - last one is start of next step.
static uint8_t microstepcurve[] = {0,   25,  50,  74,  98,  120, 141, 162, 180,
                                   197, 212, 225, 236, 244, 250, 253, 255};
#endif

/**************************************************************************/
/*!
    @brief  Initialize the I2C hardware and PWM driver, then turn off all pins.
    @param    freq
    The PWM frequency for the driver, used for speed control and microstepping.
    By default we use 1600 Hz which is a little audible but efficient.
    @param    theWire
    A pointer to an optional I2C interface. If not provided, we use Wire or
   Wire1 (on Due)
    @returns true if successful, false otherwise
*/
/**************************************************************************/
template <size_t numDrivers>
bool Adafruit_MotorShield<numDrivers>::Init(Adafruit_MotorShield<numDrivers>::Config config) {
  // init PWM w/_freq
  // pwm_ = Adafruit_MS_PWMServoDriver(_addr);
  pwm_.Init(config.i2c_config.periph, config.addresses, config.dma_buffer_a, config.dma_buffer_b, config.oe_pin);
  for (uint8_t i = 0; i < 16; i++)
    pwm_.setPWM(i, 0, 0);
  return true;
}

/**************************************************************************/
/*!
    @brief  Helper that sets the PWM output on a pin and manages 'all on or off'
    @param  pin The PWM output on the driver that we want to control (0-15)
    @param  value The 12-bit PWM value we want to set (0-4095) - 4096 is a
   special 'all on' value
*/
/**************************************************************************/
template <size_t numDrivers>
void Adafruit_MotorShield<numDrivers>::setPWM(uint8_t pin, uint16_t value) {
  if (value > 4095) {
    pwm_.setPWM(pin, 4096, 0);
  } else
    pwm_.setPWM(pin, 0, value);
}

/**************************************************************************/
/*!
    @brief  Helper that sets the PWM output on a pin as if it were a GPIO
    @param  pin The PWM output on the driver that we want to control (0-15)
    @param  value HIGH or LOW depending on the value you want!
*/
/**************************************************************************/
template <size_t numDrivers>
void Adafruit_MotorShield<numDrivers>::setPin(uint8_t pin, bool value) {
  if (value == 0)
    pwm_.SetLed(pin, (uint8_t) 0);
  else
    pwm_.SetLed(pin, (uint8_t) 255);
}

/**************************************************************************/
/*!
    @brief  Mini factory that will return a pointer to an already-allocated
    Adafruit_DCMotor object. Initializes the DC motor and turns off all pins
    @param  num The DC motor port we want to use: 1 thru 4 are valid
    @returns NULL if something went wrong, or a pointer to a Adafruit_DCMotor
*/
/**************************************************************************/
template <size_t numDrivers>
Adafruit_MotorShield<numDrivers>::Adafruit_DCMotor *Adafruit_MotorShield<numDrivers>::getMotor(uint8_t num) {
  if (num > 4)
    return NULL;

  num--;

  if (dcmotors[num].motornum == 0) {
    // not init'd yet!
    dcmotors[num].motornum = num;
    dcmotors[num].MC = this;
    uint8_t pwm, in1, in2;
    if (num == 0) {
      pwm = 8;
      in2 = 9;
      in1 = 10;
    } else if (num == 1) {
      pwm = 13;
      in2 = 12;
      in1 = 11;
    } else if (num == 2) {
      pwm = 2;
      in2 = 3;
      in1 = 4;
    } else if (num == 3) {
      pwm = 7;
      in2 = 6;
      in1 = 5;
    }
    dcmotors[num].PWMpin = pwm;
    dcmotors[num].IN1pin = in1;
    dcmotors[num].IN2pin = in2;
  }
  return &dcmotors[num];
}

/**************************************************************************/
/*!
    @brief  Mini factory that will return a pointer to an already-allocated
    Adafruit_StepperMotor object with a given 'steps per rotation.
    Then initializes the stepper motor and turns off all pins.
    @param  steps How many steps per revolution (used for RPM calculation)
    @param  num The stepper motor port we want to use: only 1 or 2 are valid
    @returns NULL if something went wrong, or a pointer to a
   Adafruit_StepperMotor
*/
/**************************************************************************/
template <size_t numDrivers>
Adafruit_MotorShield<numDrivers>::Adafruit_StepperMotor 
  *Adafruit_MotorShield<numDrivers>::getStepper(uint16_t steps,
                                                uint8_t num) {
  if (num > 2)
    return NULL;

  num--;

  if (steppers[num].steppernum == 0) {
    // not init'd yet!
    steppers[num].steppernum = num;
    steppers[num].revsteps = steps;
    steppers[num].MC = this;
    uint8_t pwma, pwmb, ain1, ain2, bin1, bin2;
    if (num == 0) {
      pwma = 8;
      ain2 = 9;
      ain1 = 10;
      pwmb = 13;
      bin2 = 12;
      bin1 = 11;
    } else if (num == 1) {
      pwma = 2;
      ain2 = 3;
      ain1 = 4;
      pwmb = 7;
      bin2 = 6;
      bin1 = 5;
    }
    steppers[num].PWMApin = pwma;
    steppers[num].PWMBpin = pwmb;
    steppers[num].AIN1pin = ain1;
    steppers[num].AIN2pin = ain2;
    steppers[num].BIN1pin = bin1;
    steppers[num].BIN2pin = bin2;
  }
  return &steppers[num];
}

/******************************************
               MOTORS
******************************************/

/**************************************************************************/
/*!
    @brief  Create a DCMotor object, un-initialized!
    You should never call this, instead have the {@link Adafruit_MotorShield}
    give you a DCMotor object with {@link Adafruit_MotorShield.getMotor}
*/
/**************************************************************************/
template <size_t numDrivers>
Adafruit_MotorShield<numDrivers>::Adafruit_DCMotor::Adafruit_DCMotor(void) {
  MC = NULL;
  motornum = 0;
  PWMpin = IN1pin = IN2pin = 0;
}

/**************************************************************************/
/*!
    @brief  Control the DC Motor direction and action
    @param  cmd The action to perform, can be FORWARD, BACKWARD or RELEASE
*/
/**************************************************************************/
template <size_t numDrivers>
void Adafruit_MotorShield<numDrivers>::Adafruit_DCMotor::run(uint8_t cmd) {
  switch (cmd) {
  case FORWARD:
    MC->setPin(IN2pin, LOW); // take low first to avoid 'break'
    MC->setPin(IN1pin, HIGH);
    break;
  case BACKWARD:
    MC->setPin(IN1pin, LOW); // take low first to avoid 'break'
    MC->setPin(IN2pin, HIGH);
    break;
  case RELEASE:
    MC->setPin(IN1pin, LOW);
    MC->setPin(IN2pin, LOW);
    break;
  }
}

/**************************************************************************/
/*!
    @brief  Control the DC Motor speed/throttle
    @param  speed The 8-bit PWM value, 0 is off, 255 is on
*/
/**************************************************************************/
template <size_t numDrivers>
void Adafruit_MotorShield<numDrivers>::Adafruit_DCMotor::setSpeed(uint8_t speed) {
  MC->setPWM(PWMpin, speed * 16);
}

/**************************************************************************/
/*!
    @brief  Control the DC Motor speed/throttle with 12 bit resolution.
    @param  speed The 12-bit PWM value, 0 (full off) to 4095 (full on)
*/
/**************************************************************************/
template <size_t numDrivers>
void Adafruit_MotorShield<numDrivers>::Adafruit_DCMotor::setSpeedFine(uint16_t speed) {
  MC->setPWM(PWMpin, speed > 4095 ? 4095 : speed);
}

/**************************************************************************/
/*!
    @brief  Set DC motor to full on.
*/
/**************************************************************************/
template <size_t numDrivers>
void Adafruit_MotorShield<numDrivers>::Adafruit_DCMotor::fullOn() { MC->pwm_.setPWM(PWMpin, 4096, 0); }

/**************************************************************************/
/*!
    @brief  Set DC motor to full off.
*/
/**************************************************************************/
template <size_t numDrivers>
void Adafruit_MotorShield<numDrivers>::Adafruit_DCMotor::fullOff() { MC->pwm_.setPWM(PWMpin, 0, 4096); }

/******************************************
               STEPPERS
******************************************/

/**************************************************************************/
/*!
    @brief  Create a StepperMotor object, un-initialized!
    You should never call this, instead have the {@link Adafruit_MotorShield}
    give you a StepperMotor object with {@link Adafruit_MotorShield.getStepper}
*/
/**************************************************************************/
template <size_t numDrivers>
Adafruit_MotorShield<numDrivers>::Adafruit_StepperMotor::Adafruit_StepperMotor(void) {
  revsteps = steppernum = currentstep = 0;
}

/**************************************************************************/
/*!
    @brief  Set the delay for the Stepper Motor speed in RPM
    @param  rpm The desired RPM, we will do our best to reach it!
*/
/**************************************************************************/
template <size_t numDrivers>
void Adafruit_MotorShield<numDrivers>::Adafruit_StepperMotor::setSpeed(uint16_t rpm) {
  // Serial.println("steps per rev: "); Serial.println(revsteps);
  // Serial.println("RPM: "); Serial.println(rpm);

  usperstep = 60000000 / ((uint32_t)revsteps * (uint32_t)rpm);
}

/**************************************************************************/
/*!
    @brief  Release all pins of the stepper motor so it free-spins
*/
/**************************************************************************/
template <size_t numDrivers>
void Adafruit_MotorShield<numDrivers>::Adafruit_StepperMotor::release(void) {
  MC->setPin(AIN1pin, LOW);
  MC->setPin(AIN2pin, LOW);
  MC->setPin(BIN1pin, LOW);
  MC->setPin(BIN2pin, LOW);
  MC->setPWM(PWMApin, 0);
  MC->setPWM(PWMBpin, 0);
}

/**************************************************************************/
/*!
    @brief  Move the stepper motor with the given RPM speed, don't forget to
   call
    {@link Adafruit_StepperMotor.setSpeed} to set the speed!
    @param  steps The number of steps we want to move
    @param  dir The direction to go, can be FORWARD or BACKWARD
    @param  style How to perform each step, can be SINGLE, DOUBLE, INTERLEAVE or
   MICROSTEP
*/
/**************************************************************************/
template <size_t numDrivers>
void Adafruit_MotorShield<numDrivers>::Adafruit_StepperMotor::step(uint16_t steps, uint8_t dir, uint8_t style) {
  uint32_t uspers = usperstep;

  if (style == INTERLEAVE) {
    uspers /= 2;
  } else if (style == MICROSTEP) {
    uspers /= MICROSTEPS;
    steps *= MICROSTEPS;
  }

  while (steps--) {
    // Serial.println("step!"); Serial.println(uspers);
    onestep(dir, style);
    delayMicroseconds(uspers);
  }
}

/**************************************************************************/
/*!
    @brief  Move the stepper motor one step only, with no delays
    @param  dir The direction to go, can be FORWARD or BACKWARD
    @param  style How to perform each step, can be SINGLE, DOUBLE, INTERLEAVE or
   MICROSTEP
    @returns The current step/microstep index, useful for
   Adafruit_StepperMotor.step to keep track of the current location, especially
   when microstepping
*/
/**************************************************************************/
template <size_t numDrivers>
uint8_t Adafruit_MotorShield<numDrivers>::Adafruit_StepperMotor::onestep(uint8_t dir, uint8_t style) {
  uint8_t ocrb, ocra;

  ocra = ocrb = 255;

  // next determine what sort of stepping procedure we're up to
  if (style == SINGLE) {
    if ((currentstep / (MICROSTEPS / 2)) % 2) { // we're at an odd step, weird
      if (dir == FORWARD) {
        currentstep += MICROSTEPS / 2;
      } else {
        currentstep -= MICROSTEPS / 2;
      }
    } else { // go to the next even step
      if (dir == FORWARD) {
        currentstep += MICROSTEPS;
      } else {
        currentstep -= MICROSTEPS;
      }
    }
  } else if (style == DOUBLE) {
    if (!(currentstep / (MICROSTEPS / 2) % 2)) { // we're at an even step, weird
      if (dir == FORWARD) {
        currentstep += MICROSTEPS / 2;
      } else {
        currentstep -= MICROSTEPS / 2;
      }
    } else { // go to the next odd step
      if (dir == FORWARD) {
        currentstep += MICROSTEPS;
      } else {
        currentstep -= MICROSTEPS;
      }
    }
  } else if (style == INTERLEAVE) {
    if (dir == FORWARD) {
      currentstep += MICROSTEPS / 2;
    } else {
      currentstep -= MICROSTEPS / 2;
    }
  }

  if (style == MICROSTEP) {
    if (dir == FORWARD) {
      currentstep++;
    } else {
      // BACKWARDS
      currentstep--;
    }

    currentstep += MICROSTEPS * 4;
    currentstep %= MICROSTEPS * 4;

    ocra = ocrb = 0;
    if (currentstep < MICROSTEPS) {
      ocra = microstepcurve[MICROSTEPS - currentstep];
      ocrb = microstepcurve[currentstep];
    } else if ((currentstep >= MICROSTEPS) && (currentstep < MICROSTEPS * 2)) {
      ocra = microstepcurve[currentstep - MICROSTEPS];
      ocrb = microstepcurve[MICROSTEPS * 2 - currentstep];
    } else if ((currentstep >= MICROSTEPS * 2) &&
               (currentstep < MICROSTEPS * 3)) {
      ocra = microstepcurve[MICROSTEPS * 3 - currentstep];
      ocrb = microstepcurve[currentstep - MICROSTEPS * 2];
    } else if ((currentstep >= MICROSTEPS * 3) &&
               (currentstep < MICROSTEPS * 4)) {
      ocra = microstepcurve[currentstep - MICROSTEPS * 3];
      ocrb = microstepcurve[MICROSTEPS * 4 - currentstep];
    }
  }

  currentstep += MICROSTEPS * 4;
  currentstep %= MICROSTEPS * 4;

  MC->setPWM(PWMApin, ocra * 16);
  MC->setPWM(PWMBpin, ocrb * 16);

  // release all
  uint8_t latch_state = 0; // all motor pins to 0

  // Serial.println(step, DEC);
  if (style == MICROSTEP) {
    if (currentstep < MICROSTEPS)
      latch_state |= 0x03;
    if ((currentstep >= MICROSTEPS) && (currentstep < MICROSTEPS * 2))
      latch_state |= 0x06;
    if ((currentstep >= MICROSTEPS * 2) && (currentstep < MICROSTEPS * 3))
      latch_state |= 0x0C;
    if ((currentstep >= MICROSTEPS * 3) && (currentstep < MICROSTEPS * 4))
      latch_state |= 0x09;
  } else {
    switch (currentstep / (MICROSTEPS / 2)) {
    case 0:
      latch_state |= 0x1; // energize coil 1 only
      break;
    case 1:
      latch_state |= 0x3; // energize coil 1+2
      break;
    case 2:
      latch_state |= 0x2; // energize coil 2 only
      break;
    case 3:
      latch_state |= 0x6; // energize coil 2+3
      break;
    case 4:
      latch_state |= 0x4; // energize coil 3 only
      break;
    case 5:
      latch_state |= 0xC; // energize coil 3+4
      break;
    case 6:
      latch_state |= 0x8; // energize coil 4 only
      break;
    case 7:
      latch_state |= 0x9; // energize coil 1+4
      break;
    }
  }

  if (latch_state & 0x1) {
    // Serial.println(AIN2pin);
    MC->setPin(AIN2pin, HIGH);
  } else {
    MC->setPin(AIN2pin, LOW);
  }
  if (latch_state & 0x2) {
    MC->setPin(BIN1pin, HIGH);
    // Serial.println(BIN1pin);
  } else {
    MC->setPin(BIN1pin, LOW);
  }
  if (latch_state & 0x4) {
    MC->setPin(AIN1pin, HIGH);
    // Serial.println(AIN1pin);
  } else {
    MC->setPin(AIN1pin, LOW);
  }
  if (latch_state & 0x8) {
    MC->setPin(BIN2pin, HIGH);
    // Serial.println(BIN2pin);
  } else {
    MC->setPin(BIN2pin, LOW);
  }

  return currentstep;
}

#endif // DSY_MOTOR_H

} // namespace daisy