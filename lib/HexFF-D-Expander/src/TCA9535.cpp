
#include "HexFF/TCA9535.h"

TCA9535::TCA9535() {}

// Copy a pointer to the I2C interface that we're using anc copy the address for this device
bool TCA9535::begin(TwoWire* interface, uint8_t address) {
  _pInterface = interface;
  _address = address;
  // Define all pins as inputs
  if (!TCA9535::_writeRegisters(TCA9535_CFG0, 0xFFFF)) {
    return false;
  }
  // Define all outputs as low
  if (!TCA9535::_writeRegisters(TCA9535_OUT0, 0x0000)) {
    return false;
  }
  return true;
}

bool TCA9535::_writeRegister(uint8_t reg, uint8_t value) {
  _pInterface->beginTransmission(_address);
  _pInterface->write(reg);
  _pInterface->write(value);
  int r = _pInterface->endTransmission();
  if (r == 0) {
    // success
    _shadowRegisters[reg] = value;  // copy value into the shadow register
    return true;
  }
  Serial.print("WriteRegister - endTransmission expecting 0; returned ");
  Serial.println(r);
  return false;
}

bool TCA9535::_writeRegisters(uint8_t reg, uint16_t value) {
  _pInterface->beginTransmission(_address);
  _pInterface->write(reg);
  _pInterface->write(value & 0x00FF);  // low byte
  _pInterface->write(value >> 8);      // high byte
  int r = _pInterface->endTransmission();
  if (r == 0) {
    // success
    // copy value into the shadow registers
    _shadowRegisters[reg] = (value & 0xFF);
    _shadowRegisters[reg + 1] = (value >> 8);
    return true;
  }
  Serial.print("WriteRegisters - endTransmission expecting 0; returned ");
  Serial.println(r);
  return false;
}

bool TCA9535::_readRegister(uint8_t reg, uint8_t *value) {
  _pInterface->beginTransmission(_address);
  // send the register number in a write command
  _pInterface->write(reg);
  int r = _pInterface->endTransmission(false);  // this should send a repeated start
  if (r == 0) {
    // success
    // read back from device
    uint8_t nBytes = 1;
    r = _pInterface->requestFrom(_address, nBytes);  // read 1 byte from device
    if (r != nBytes) {
      Serial.print("readRegister - requestFrom expecting " + String(nBytes) + " returned ");
      Serial.println(r);
      return false;
    }
    // read 1 byte and put it into the caller's buffer
    uint8_t readVal = _pInterface->read();
    _shadowRegisters[reg] = readVal;  // copy value into the shadow register
    *value = readVal;
    return true;
  } else {
    // command failed
    Serial.print("readRegister - endTransmission expecting 0; returned ");
    Serial.println(r);
    return false;
  }
}

bool TCA9535::_readRegisters(uint8_t reg, uint16_t *value) {
  _pInterface->beginTransmission(_address);
  // send the register number in a write command
  _pInterface->write(reg);
  int r = _pInterface->endTransmission(false);  // this should send a repeated start
  if (r == 0) {
    // success
    // read back from device
    uint8_t nBytes = 2;
    r = _pInterface->requestFrom(_address, nBytes);  // read 1 byte from device
    if (r != nBytes) {
      Serial.print("readRegisters - requestFrom expecting " + String(nBytes) + " returned ");
      Serial.println(r);
      return false;
    }
    // read 1 byte and put it into the caller's buffer
    uint16_t readVal_L = _pInterface->read();  // this is the 1st byte going into the low half
    _shadowRegisters[reg] = readVal_L;         // copy value into the shadow register
    uint16_t readVal_H = _pInterface->read();
    _shadowRegisters[reg + 1] = readVal_H;  // copy value into the shadow register
    readVal_L += (readVal_H << 8);          // combine both byes into a 16-bit word
    *value = readVal_L;
    return true;
  } else {
    // command failed
    Serial.print("readRegisters - endTransmission expecting 0; returned ");
    Serial.println(r);
    return false;
  }
}

// returns the value that was last written to an output pin
bool TCA9535::readOutputPin(uint8_t pinNumber) {
  uint16_t regShadow16
      = _shadowRegisters[TCA9535_OUT0] + (((uint16_t)_shadowRegisters[TCA9535_OUT1]) << 8);
  return (regShadow16 & (((uint16_t)1) << pinNumber));
}

// ledcWrite(PWMChannel, dutyCycle)

// Writes the specified value to an output pin
int TCA9535::writePin(uint8_t pinNumber, bool value) {
  uint16_t regShadow16
      = _shadowRegisters[TCA9535_OUT0] + (((uint16_t)_shadowRegisters[TCA9535_OUT1]) << 8);
  if ((regShadow16 & (1 << pinNumber)) == value) {
    return 1;  // we're being asked to write the same value we last wrote to this bit. No point in
               // accessing the bus for this
  }
  if (value) {
    setPin(pinNumber);
    return 2;
  } else {
    clrPin(pinNumber);
    return 3;
  }
}

// Turns on a specified output pin
void TCA9535::setPin(uint8_t pinNumber) {
  uint16_t regShadow16
      = _shadowRegisters[TCA9535_OUT0] + (((uint16_t)_shadowRegisters[TCA9535_OUT1]) << 8);
  if (regShadow16 & (1 << pinNumber))
    return;  // skip the access to the bus if the bit is already On
  if (pinNumber < 8) {
    TCA9535::_writeRegister(TCA9535_OUT0, _shadowRegisters[TCA9535_OUT0] | (1 << pinNumber));
  } else {
    TCA9535::_writeRegister(TCA9535_OUT1, _shadowRegisters[TCA9535_OUT1] | (1 << (pinNumber - 8)));
  }
}

// Turns off a specified output pin
void TCA9535::clrPin(uint8_t pinNumber) {
  uint16_t regShadow16
      = _shadowRegisters[TCA9535_OUT0] + (((uint16_t)_shadowRegisters[TCA9535_OUT1]) << 8);
  if ((regShadow16 & (1 << pinNumber)) == 0)
    return;  // skip the access to the bus if the bit is already Off
  if (pinNumber < 8) {
    TCA9535::_writeRegister(TCA9535_OUT0, _shadowRegisters[TCA9535_OUT0] & ~(1 << pinNumber));
  } else {
    TCA9535::_writeRegister(TCA9535_OUT1, _shadowRegisters[TCA9535_OUT1] & ~(1 << (pinNumber - 8)));
  }
}

// defines a specifed pin as either input or output
void TCA9535::setPinMode(uint8_t pinNumber, uint8_t mode) {
  if (mode == OUTPUT) {
    // write 0 to corresponding bit for output
    if (pinNumber < 8) {
      TCA9535::_writeRegister(TCA9535_CFG0, _shadowRegisters[TCA9535_CFG0] & ~(1 << pinNumber));
    } else {
      TCA9535::_writeRegister(TCA9535_CFG1,
                              _shadowRegisters[TCA9535_CFG1] & ~(1 << (pinNumber - 8)));
    }
  } else {
    // write 1 to corresponding bit for input
    if (pinNumber < 8) {
      TCA9535::_writeRegister(TCA9535_CFG0, _shadowRegisters[TCA9535_CFG0] | (1 << pinNumber));
    } else {
      TCA9535::_writeRegister(TCA9535_CFG1,
                              _shadowRegisters[TCA9535_CFG1] | (1 << (pinNumber - 8)));
    }
  }
}

// Reads an input
bool TCA9535::readInputPin(uint8_t pinNumber) {
  uint8_t value = 0;
  if (pinNumber < 8) {
    TCA9535::_readRegister(TCA9535_INP0, &value);
    value &= (1 << pinNumber);
  } else {
    TCA9535::_readRegister(TCA9535_INP1, &value);
    value &= (1 << (pinNumber - 8));
  }
  return value;
}

// returns the shadow of hte output register if the pin has been defined as output, or the actual
// pin value if definesd as input
bool TCA9535::readPin(uint8_t pinNumber) {
  uint16_t regShadow16
      = _shadowRegisters[TCA9535_CFG0] + (((uint16_t)_shadowRegisters[TCA9535_CFG1]) << 8);
  if (regShadow16 & (((uint16_t)1) << pinNumber)) {
    // cfg bit is 1 (input)
    return readInputPin(pinNumber);
  } else {
    // bit is defined as output
    return readOutputPin(pinNumber);
  }
}

// copies the bits defined by 1's in the mask from value to the output registers. Bits with 0 in the
// mask remain unchanged
void TCA9535::writePins(uint16_t pinMask, uint16_t value) {
  uint16_t lastWritten
      = _shadowRegisters[TCA9535_OUT0] + (((uint16_t)_shadowRegisters[TCA9535_OUT1]) << 8);
  lastWritten &= ~pinMask;  // turn off all bits specified in the mask
  lastWritten
      |= (pinMask & value);  // turn on the bits that are On both in the mask and in the value
  TCA9535::_writeRegisters(TCA9535_OUT0, lastWritten);
}

// reads both input registers (16-bits) and returns the value as a 16-bit word. It also updates the
// Inputs[] array
uint16_t TCA9535::readInputs() {
  uint16_t inputRegisters;
  TCA9535::_readRegisters(TCA9535_INP0, &inputRegisters);
  uint16_t mask = 0x0001;
  int i = 0;
  while (mask != 0) {
    this->Inputs[i] = (inputRegisters & mask);
    i++;
    mask = (mask << 1);
  }
  return inputRegisters;
}
