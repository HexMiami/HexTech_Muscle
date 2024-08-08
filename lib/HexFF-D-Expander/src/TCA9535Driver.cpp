#include "HexFF/TCA9535Driver.h"
#include <charconv>

#include "HexFF/HexResources.h"
#include "HexFF/I2CMasterDriver.h"

namespace HexFF {

TCA9535Driver::TCA9535Driver(const char* t_name, uint8_t t_i2c_addr, TwoWire* t_i2c, SetupCallback t_setup_callback)
    : TCA9535(), name(t_name), i2c_addr_(t_i2c_addr), i2c_(t_i2c), setupCallback_(std::move(t_setup_callback)) {}

//=======

uint16_t TCA9535Driver::read() {
  values_ = readInputs();
  return values_;
}

//=======

bool TCA9535Driver::customInit() {
  // Initialize the port expanders on the internal I2C bus
  if (!begin(i2c_, i2c_addr_)) {
    log_e("TCA9535 driver '%s' setup failed.", getName());
    return false;
  }

//  values_ = readInputs();
  read();

  return setupCallback_(*this);
}

//=======

bool TCA9535Driver::customRefresh() {
//  values_ = readInputs();
  read();
  return false;
}

//=======

const char* TCA9535Driver::getName() {
  return name.c_str();
}

//=======

TCA9535Driver* FactoryTCA9535Driver::create(const char* t_name, const JsonVariant& t_settings,
                                            const JsonVariant& t_extra_settings) {
  if (t_settings.isNull()) {
    return nullptr;
  }

  auto i2c_driver_name = t_settings["i2c_driver"].as<const char*>();
  auto i2c_addr = t_settings["i2c_addr"];

  if (i2c_driver_name == nullptr || !i2c_addr.is<uint8_t>()) {
    return nullptr;
  }

  auto i2c_driver = reinterpret_cast<I2CMasterDriver*>(HexResources::getHexBoard()->getDriver(i2c_driver_name));
  if (i2c_driver == nullptr) {
    log_e("I2C driver '%s' does not exist.", i2c_driver_name);
    return nullptr;
  }

  auto pins = t_settings["pins"].as<JsonObject>();
  std::vector<std::pair<uint8_t, uint8_t>> pin_values;
  pin_values.reserve(pins.size());

  for (JsonPair item : pins) {
    uint8_t pin = 0xFF;
    std::string_view pin_key = item.key().c_str();
    std::from_chars(pin_key.begin(), pin_key.end(), pin);
    auto value = item.value();
    if (pin > 15 || !value.is<uint8_t>()) {
      log_e("TCA9535 driver '%s' has a bad pin_key/value setting.", t_name);
      return nullptr;
    }

    pin_values.emplace_back(pin, value);
  }

  return new TCA9535Driver(t_name, i2c_addr, i2c_driver, [pin_values](TCA9535Driver& tt_tca) {
    for (int i = 0; i < 16; ++i) {
      tt_tca.setPinMode(i, INPUT);
    }

    for (const auto& item : pin_values) {
      tt_tca.setPinMode(item.first, OUTPUT);
      tt_tca.writePin(item.first, item.second);
    }
    return true;
  });
}

//=======

PinOutputTCA9535Driver::PinOutputTCA9535Driver(const char* t_name, uint8_t t_pin, TCA9535Driver* t_tca,
                                               uint32_t t_off_value, uint32_t t_on_value)
    : PinOutputDriver(t_name, t_pin, t_off_value, t_on_value), tca{t_tca} {}

//=======

bool PinOutputTCA9535Driver::customInit() {
  tca->setPinMode(pin, OUTPUT);
  tca->writePin(pin, offValue_);

  return true;
}

//=======

void PinOutputTCA9535Driver::write(uint32_t _value) {
  tca->writePin(pin, _value);
}

//=======

uint32_t PinOutputTCA9535Driver::read() {
  value = tca->readPin(pin);
  return value;
}

//=======

PinInputDriver* FactoryPinInputTCA9535Driver::create(const char* t_name, const JsonVariant& t_settings,
                                                     const JsonVariant&) {
  if (t_settings.isNull()) {
    return nullptr;
  }

  auto tca9535_driver_name = t_settings["tca9535_driver"].as<const char*>();
  auto pin = t_settings["pin"];

  if (tca9535_driver_name == nullptr || !pin.is<uint8_t>()) {
    return nullptr;
  }

  auto tca9535_driver = reinterpret_cast<TCA9535Driver*>(HexResources::getHexBoard()->getDriver(tca9535_driver_name));
  if (tca9535_driver == nullptr) {
    log_e("TCA9535 driver '%s' does not exist.", tca9535_driver_name);
    return nullptr;
  }

  return new PinInputDriver(
      t_name, pin,
      [tca9535_driver](uint8_t tt_pin) {
        tca9535_driver->setPinMode(tt_pin, INPUT);
        return true;
      },
      [tca9535_driver](uint8_t tt_pin) { return tca9535_driver->readPin(tt_pin); });
}

//=======

PinOutputTCA9535Driver* FactoryPinOutputTCA9535Driver::create(const char* t_name, const JsonVariant& t_settings,
                                                              const JsonVariant&) {
  if (t_settings.isNull()) {
    return nullptr;
  }

  auto tca9535_driver_name = t_settings["tca9535_driver"].as<const char*>();
  auto pin = t_settings["pin"];

  if (tca9535_driver_name == nullptr || !pin.is<uint8_t>()) {
    return nullptr;
  }

  uint8_t off_value = t_settings["off_value"] | 0u;
  uint8_t on_value = t_settings["on_value"] | 1u;

  auto tca9535_driver = reinterpret_cast<TCA9535Driver*>(HexResources::getHexBoard()->getDriver(tca9535_driver_name));
  if (tca9535_driver == nullptr) {
    log_e("TCA9535 driver '%s' does not exist.", tca9535_driver_name);
    return nullptr;
  }

  return new PinOutputTCA9535Driver(t_name, pin.as<uint8_t>(), tca9535_driver, off_value, on_value);
}

}  // namespace HexFF
