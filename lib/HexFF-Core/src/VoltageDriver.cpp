#include "HexFF/VoltageDriver.h"

namespace HexFF {

VoltageDriver::VoltageDriver(uint8_t _pin, double _volt_ratio) : BasePinReaderDriver(_pin), volt_ratio_{_volt_ratio} {}

//=======

const char* VoltageDriver::getName() {
  return "volt";
}

//=======

double VoltageDriver::operator()() const {
  return double(analogReadMilliVolts(pin_)) / 1000 / volt_ratio_;
}

//=======

VoltageDriver* FactoryVoltageDriver::create(const char*, const JsonVariant& t_settings, const JsonVariant&) {
  if (t_settings.isNull()) {
    return nullptr;
  }

  auto pin = t_settings["pin"];
  if (!pin.is<uint8_t>()) {
    return nullptr;
  }

  auto ratio = t_settings["ratio"] | 0.025;

  return new VoltageDriver(pin, ratio);
}

}  // namespace HexFF
