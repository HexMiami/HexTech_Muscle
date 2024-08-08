#pragma once

#include <functional>

#include <Wire.h>

#include "HexFF/Driver.h"
#include "HexFF/PinInputDriver.h"
#include "HexFF/PinOutputDriver.h"

#include "TCA9535.h"

namespace HexFF {

class TCA9535Driver : public Driver, public TCA9535 {
 public:
  using SetupCallback = std::function<bool(TCA9535Driver&)>;

 protected:
  const String name;
  uint8_t i2c_addr_;
  TwoWire* i2c_;
  SetupCallback setupCallback_;  // TODO Remove callback. It is not used with self-register factory

  uint16_t values_;

  bool customInit() override;
  bool customRefresh() override;

 public:
  TCA9535Driver(const char* t_name, uint8_t t_i2c_addr, TwoWire* t_i2c,
                SetupCallback t_setup_callback = defaultSetupCallback);

  const char* getName() override;

  uint16_t read();

  constexpr static bool defaultSetupCallback(const TCA9535Driver&) { return true; };

  const uint16_t& values = values_;
};

//=======

class FactoryTCA9535Driver {
 public:
  static TCA9535Driver* create(const char* t_name, const JsonVariant& t_settings, const JsonVariant& t_extra_settings);

 private:
  static inline bool s_registered = FactoryDriver::add("TCA9535", create);
};

//=======

class PinOutputTCA9535Driver : public PinOutputDriver {
 protected:
  TCA9535Driver* tca;

  bool customInit() override;

 public:
  PinOutputTCA9535Driver(const char* t_name, uint8_t t_pin, TCA9535Driver* t_tca, uint32_t t_off_value = LOW,
                         uint32_t t_on_value = HIGH);

  void write(uint32_t value) override;
  uint32_t read() override;
};

//=======

//TODO Create a specialized PinInput driver for TCA9535 that use the values previously read, avoiding a new reading
class FactoryPinInputTCA9535Driver {
 public:
  static PinInputDriver* create(const char* t_name, const JsonVariant& t_settings, const JsonVariant& t_extra_settings);

 private:
  static inline bool s_registered = FactoryDriver::add("PInTCA9535", create);
};

//=======

class FactoryPinOutputTCA9535Driver {
 public:
  static PinOutputTCA9535Driver* create(const char* t_name, const JsonVariant& t_settings,
                                        const JsonVariant& t_extra_settings);

 private:
  static inline bool s_registered = FactoryDriver::add("POutTCA9535", create);
};

}  // namespace HexFF