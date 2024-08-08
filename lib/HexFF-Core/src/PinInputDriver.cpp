#include "HexFF/PinInputDriver.h"

#include <esp32-hal-log.h>

namespace HexFF {

bool PinInputDriver::PIN_INPUT_SETUP_DEFAULT(uint8_t _pin) {
  pinMode(_pin, INPUT);
  return true;
};

//=======

uint16_t PinInputDriver::PIN_INPUT_READ_DEFAULT(uint8_t _pin) {
  return analogRead(_pin);
}

//=======

const char* PinInputDriver::getName() {
  return name.c_str();
};

//=======

PinInputDriver::PinInputDriver(const char* _name, uint8_t _pin, std::function<bool(uint8_t)> _setup,
                               std::function<uint16_t(uint8_t)> _read)
    : name(_name), pin(_pin), setup{_setup}, read{_read} {}

//=======

bool PinInputDriver::customInit() {
  if (setup(pin)) {
    value = this->read(pin);
    return true;
  }
  return false;
}

//=======

bool PinInputDriver::customRefresh() {
  auto prevValue = value;
  value = this->read(pin);
  if (prevValue != value) {
    Serial.println(String(name) + " | " + prevValue + " | " + value);
  }
  return false;
}

//=======

bool PinInputDriver::customSet(const String& action) {
  if (action.compareTo("fakeHIGH") == 0) {
    value = HIGH;
  } else if (action.compareTo("fakeLOW") == 0) {
    value = LOW;
  } else {
    return false;
  }
  return true;
}

//=======

uint16_t PinInputDriver::getValue() {
  return value;
}

//=======

void PinInputDriver::customReadStatus(JsonVariant doc, bool compressed) {
  if (compressed) {
    String data(static_cast<const char*>(doc["data"]));
    data += ':';
    data += value;
    doc["data"] = data;
  } else {
    doc["value"] = value;
  }
}

//=======

PinInputDriver* FactoryPinInputDriver::create(const char* t_name, const JsonVariant& t_settings, const JsonVariant&) {
  if (t_settings.isNull()) {
    return nullptr;
  }

  auto pin = t_settings["pin"];
  if (!pin.is<uint8_t>()) {
    return nullptr;
  }

  return new PinInputDriver(
      t_name, pin,
      [](uint8_t tt_pin) {
        pinMode(tt_pin, OUTPUT);
        return true;
      },
      [](uint8_t tt_pin) { return digitalRead(tt_pin); });
}

//=======

PinInputDriver* FactoryPinInputAnalogDriver::create(const char* t_name, const JsonVariant& t_settings,
                                                    const JsonVariant&) {
  if (t_settings.isNull()) {
    return nullptr;
  }

  auto pin = t_settings["pin"];
  if (!pin.is<uint8_t>()) {
    return nullptr;
  }

  return new PinInputDriver(
      t_name, pin,
      [](uint8_t tt_pin) {
        pinMode(tt_pin, OUTPUT);
        return true;
      },
      [](uint8_t tt_pin) { return analogRead(tt_pin); });
}

}  // namespace HexFF
