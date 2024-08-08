#include "HexFF/SPIDriver.h"

namespace HexFF {

SPIDriver::SPIDriver(const char* _name, uint8_t _clk, uint8_t _miso, uint8_t _mosi, uint32_t _frequency)
    : SPIClass(), name(_name), clk(_clk), miso(_miso), mosi(_mosi), frequency(_frequency) {}

//=======

const char* SPIDriver::getName() {
  return name.c_str();
}

//=======

bool SPIDriver::customInit() {
  begin(static_cast<int8_t>(clk), static_cast<int8_t>(miso), static_cast<int8_t>(mosi), -1);
  setFrequency(frequency);
  return true;
}

//=======

SPIDriver* FactorySPIDriver::create(const char* t_name, const JsonVariant& t_settings,
                                    const JsonVariant& t_extra_settings) {
  if (t_settings.isNull()) {
    return nullptr;
  }

  auto clk = t_settings["clk"];
  auto miso = t_settings["miso"];
  auto mosi = t_settings["mosi"];
  auto frequency = t_settings["frequency"];
  if (!clk.is<uint8_t>() || !miso.is<uint8_t>() || !mosi.is<uint8_t>() || !frequency.is<uint32_t>()) {
    return nullptr;
  }

  return new SPIDriver(t_name, clk, miso, mosi, frequency);
}

}  // namespace HexFF
