#pragma once
#include <WString.h>
#include <Wire.h>

#include "Driver.h"

namespace HexFF {
  class I2CDriver : public Driver, public TwoWire {
  protected:
    const String name;
    uint8_t sda;
    uint8_t scl;
    uint32_t frequency;
    uint8_t slaveAddr;
    bool customInit() override;

    // size_t writeToRegister(uint8_t reg, uint8_t *content, size_t n);
    // uint8_t writeToDevice(uint8_t i2caddr, uint8_t reg, uint8_t *content, size_t n);
    // uint8_t writeReverseToDevice(uint8_t i2caddr, uint8_t reg, const uint8_t *content, size_t n);
    // bool readFromDevice(uint8_t i2caddr, uint8_t reg, uint8_t *content, size_t n);

  public:
    // slaveAddr = 0xFF -> master mode by default
    I2CDriver(const char *name, uint8_t busId, uint8_t sda, uint8_t scl, uint32_t frequency = 0, uint8_t slaveAddr = 0xFF);

    // template <typename TValue>
    // uint8_t writeToDevice(uint8_t i2caddr, uint8_t reg, const TValue &value) {
    //   constexpr const size_t n = sizeof(TValue);
    //   auto source = reinterpret_cast<const uint8_t *>(&value);
    //   return writeReverseToDevice(i2caddr, reg, source, n);
    // }

    // template <typename TValue> bool readFromDevice(uint8_t i2caddr, uint8_t reg, TValue &value) {
    //   auto destiny = reinterpret_cast<uint8_t *>(&value);
    //   return readFromDevice(i2caddr, reg, destiny, sizeof(TValue));
    // }
    const char *getName() override;
  };
}  // namespace HexFF
