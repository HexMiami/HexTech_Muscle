#include "HexFF/I2CDriver.h"

#include <esp32-hal-log.h>

namespace HexFF {

I2CDriver::I2CDriver(const char* _name, uint8_t busId, uint8_t _sda, uint8_t _scl, uint32_t _frequency,
                       uint8_t _slaveAddr)
      : TwoWire(static_cast<uint8_t>(busId)),
        name(_name),
        sda(_sda),
        scl(_scl),
        frequency(_frequency),
        slaveAddr(_slaveAddr) {}

//=======

  bool I2CDriver::customInit() {
    bool success = true;
    // if master mode
    log_v("I2CDriver::init() %s - slaveAddr = %d\n", this->getName(), slaveAddr);
    if (slaveAddr == 0xFF) {
      success = begin(static_cast<int>(sda), static_cast<int>(scl), frequency);
    } else {
      success = begin(slaveAddr, static_cast<int>(sda), static_cast<int>(scl), frequency);
    }
    if (!success) {
      log_v("I2CDriver::init() %s - slaveAddr = %d\n", this->getName(), slaveAddr);
    }
    return success;
  }

//=======

  const char *I2CDriver::getName() { return name.c_str(); }

}  // namespace HexFF
