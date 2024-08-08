#pragma once

#include <functional>

#include <WString.h>

#include "Driver.h"

namespace HexFF {

class PinInputDriver : public Driver {
 protected:
  const String name;
  const uint8_t pin;
  uint16_t value = HIGH;

  std::function<bool(uint8_t)> setup;
  std::function<uint16_t(uint8_t)> read;

  bool customInit() override;
  bool customRefresh() override;
  bool customSet(const String& action) override;
  void customReadStatus(JsonVariant doc, bool compressed = true) override;

 public:
  static bool PIN_INPUT_SETUP_DEFAULT(uint8_t _pin);
  static uint16_t PIN_INPUT_READ_DEFAULT(uint8_t _pin);
  explicit PinInputDriver(const char* name, uint8_t pin, std::function<bool(uint8_t)> setup = PIN_INPUT_SETUP_DEFAULT,
                          std::function<uint16_t(uint8_t)> read = PIN_INPUT_READ_DEFAULT);

  uint16_t getValue();

  const char* getName() override;
};

//=======

class FactoryPinInputDriver {
 public:
  static PinInputDriver* create(const char* t_name, const JsonVariant& t_settings, const JsonVariant& t_extra_settings);

 private:
  static inline bool s_registered = FactoryDriver::add("PIn", create);
};

//=======

class FactoryPinInputAnalogDriver {
 public:
  static PinInputDriver* create(const char* t_name, const JsonVariant& t_settings, const JsonVariant& t_extra_settings);

 private:
  static inline bool s_registered = FactoryDriver::add("PInAnalog", create);
};

}  // namespace HexFF