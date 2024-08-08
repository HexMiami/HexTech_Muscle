#pragma once

#include <WString.h>

#include "I2CDriver.h"

namespace HexFF {

  class I2CMasterDriver : public I2CDriver {
  protected:
    bool customSet(const String &status) override;

  public:
    I2CMasterDriver(const char *name, uint8_t busId, uint8_t sda, uint8_t scl, uint32_t frequency = 0);
    };

//=======

  class I2CMasterDriverFactory : public DriverFactory {
  public:
    Driver *create(JsonVariant configSource, const char *name) override;
  };

//=======

class FactoryI2CMasterDriver {
 public:
  static I2CMasterDriver* create(const char* t_name, const JsonVariant& t_settings,
                                 const JsonVariant& t_extra_settings);

 private:
  static inline bool s_registered = FactoryDriver::add("I2CMaster", create);
};

}  // namespace HexFF
