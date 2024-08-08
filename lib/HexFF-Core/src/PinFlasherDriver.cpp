#include "HexFF/PinFlasherDriver.h"

namespace HexFF {

  PinFlasherDriver::PinFlasherDriver(const char *_name, uint8_t _pin, unsigned long _rate)
      : PinFlasher(static_cast<int>(_pin)), rate(_rate), name(_name) {}

//=======

  bool PinFlasherDriver::customInit() {
    setOnOff(PIN_OFF);
    return true;
  }

//=======

  bool PinFlasherDriver::customSet(const String &status) {
    if (status.startsWith("flash")) {
      setOnOff(FLASH_RATE);
    } else if (status.startsWith("on")) {
      setOnOff(PIN_ON);
    } else if (status.startsWith("off")) {
      setOnOff(PIN_OFF);
    } else {
      return false;
    }
    return true;
  }

//=======

  bool PinFlasherDriver::customRefresh() {
    update();
    return false;
  }

//=======

  const char *PinFlasherDriver::getName() { return name.c_str(); }

//=======

  PinFlasherDriver* FactoryPinFlasherDriver::create(const char* t_name, const JsonVariant& t_settings,
                                                    const JsonVariant& t_extra_settings) {
    if (t_settings.isNull()) {
      return nullptr;
    }

    auto pin = t_settings["pin"];

    if (!pin.is<uint8_t>()) {
      return nullptr;
    }

    auto flash_rate = t_settings["flash_rate"] | 200u;

    return new PinFlasherDriver(t_name, pin, flash_rate);
  }

}  // namespace HexFF
