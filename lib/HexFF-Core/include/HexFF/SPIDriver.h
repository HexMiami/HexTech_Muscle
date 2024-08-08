#pragma once

#include <SPI.h>
#include <WString.h>

#include "Driver.h"

namespace HexFF {

class SPIDriver : public Driver, public SPIClass {
 public:
  SPIDriver(const char* name, uint8_t clk, uint8_t miso, uint8_t mosi, uint32_t frequency = FREQUENCY);
  const char* getName() override;

 protected:
  bool customInit() override;

  const String name;
  uint8_t clk;
  uint8_t miso;
  uint8_t mosi;
  uint32_t frequency;

  static const uint32_t FREQUENCY = 2000000;
};

//=======

class FactorySPIDriver {
 public:
  static SPIDriver* create(const char* t_name, const JsonVariant& t_settings, const JsonVariant& t_extra_settings);

 private:
  static inline bool s_registered = FactoryDriver::add("SPI", create);
};

}  // namespace HexFF
