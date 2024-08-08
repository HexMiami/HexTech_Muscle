#ifndef __TCA9535_H__
#define __TCA9535_H__

#include <Arduino.h>
#include <Wire.h>

// Register definitions
#define TCA9535_INP0 0 // input port register
#define TCA9535_INP1 1 // input port register
#define TCA9535_OUT0 2 // output port register - Resets to 1 (high)
#define TCA9535_OUT1 3 // output port register - Resets to 1 (high)
#define TCA9535_POL0 4 // input polarity inversion register - Resets to 0 (non-inverted, shouldn't be a need to touch it)
#define TCA9535_POL1 5 // input polarity inversion register - Resets to 0 (non-inverted, shouldn't be a need to touch it)
#define TCA9535_CFG0 6 // config register - Resets to 1 (input). Write 0 for output
#define TCA9535_CFG1 7 // config register - Resets to 1 (input). Write 0 for output
#define N_REGISTERS 8

class TCA9535
{
public:
  TCA9535();                                       // constructor
  bool begin(TwoWire* interface, uint8_t address); // pass a Wire interface by reference for us to use, initializes all pins as input and all output registers to 0 (low)

  void setPinMode(uint8_t pinNumber, uint8_t mode); // 0 <= bit <= 15, mode can be either INPUT or OUTPUT; INPUT_PULLUP will be treated as INPUT
  int writePin(uint8_t pinNumber, bool value);      // 0 <= bit <= 15: sets a specific output register pin as low or high
  bool readPin(uint8_t pinNumber);                  // returns the shadow of the output register if the pin has been defined as output, or the actual pin value if definesd as input

  uint16_t readInputs(); // reads both input registers (16-bits) and returns the value as a 16-bit word. It also updates the Inputs[] array
  bool Inputs[16];       // this array contains the status of the 16 input bits. It gets refreshed when readInputs() is called

  void setPin(uint8_t pinNumber); // 0 <= bit <= 15
  void clrPin(uint8_t pinNumber); // 0 <= bit <= 15

  void writePins(uint16_t pinMask, uint16_t value); // copies the bits defined by 1's in the mask from value to the output registers. Bits with 0 in the mask remain unchanged

private:
  bool _writeRegister(uint8_t reg, uint8_t value);
  bool _readRegister(uint8_t reg, uint8_t *value);
  bool _writeRegisters(uint8_t reg, uint16_t value); // writes two bytes of a word in two consecutive registers
  bool _readRegisters(uint8_t reg, uint16_t *value); // reads two consecutive 8-bit registers as a word
  bool readInputPin(uint8_t pinNumber);              // 0 <= bit <= 15 reads the input register
  bool readOutputPin(uint8_t pinNumber);             // 0 <= bit <= 15 returns the bit value from the output shadow register
  uint8_t _shadowRegisters[N_REGISTERS];
  TwoWire *_pInterface; // pointer to the I2C interface that we will be using
  uint8_t _address;
};

#endif
