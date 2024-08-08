#include "HexFF/I2CMasterDriver.h"

#include <esp32-hal-log.h>

namespace HexFF {

  constexpr static const char COMMAND_ARGS_DELIMITER = '_';

  I2CMasterDriver::I2CMasterDriver(const char *_name, uint8_t busId, uint8_t _sda, uint8_t _scl, uint32_t _frequency)
      : I2CDriver(_name, busId, _sda, _scl, _frequency) {}

//=======

  bool I2CMasterDriver::customSet(const String &action) {
    int index = action.indexOf(COMMAND_ARGS_DELIMITER);
    if (auto length = action.length(); index > 0 && index < (length - 1)) {
      auto slaveAddr = static_cast<uint8_t>(action.substring(0, index).toInt());
      const char *data = action.c_str() + index + 1;
      size_t quantity{static_cast<size_t>(length - 1 - index)};
      log_v("Writing to");
      log_v("%u", slaveAddr);
      log_v("%u", quantity);
      log_v("%s", data);
      beginTransmission(slaveAddr);
      write((const uint8_t*)data, strlen(data));
      auto result = endTransmission();
      if (result != 0) {
        log_e("I2CMasterDriver::endTransmission():%s , result = %d\n", getName(), result);
      }
    }
    return true;
  }

//=======

  Driver *I2CMasterDriverFactory::create(JsonVariant configSource, const char *name) {
    Driver *result = nullptr;
    if (name != nullptr && configSource.containsKey(name)) {
      auto driverConfig = configSource[name];
      if (driverConfig.isNull() || !driverConfig["BUS_ID"].is<uint8_t>()
          || !driverConfig["SDA_PIN"].is<uint8_t>() || !driverConfig["SCL_PIN"].is<uint8_t>()) {
        return nullptr;
      }
      auto busId = driverConfig["BUS_ID"].as<uint8_t>();
      auto sda = driverConfig["SDA_PIN"].as<uint8_t>();
      auto scl = driverConfig["SCL_PIN"].as<uint8_t>();
      result = new I2CMasterDriver(name, busId, sda, scl);
    }
    return result;
  }

//=======

I2CMasterDriver* FactoryI2CMasterDriver::create(const char* t_name, const JsonVariant& t_settings, const JsonVariant&) {
  if (t_settings.isNull()) {
    return nullptr;
  }

  auto bus_id = t_settings["bus_id"];
  auto sda_pin = t_settings["sda_pin"];
  auto scl_pin = t_settings["scl_pin"];
  auto frequency = t_settings["frequency"];

  if (!bus_id.is<uint8_t>() || !sda_pin.is<uint8_t>() || !scl_pin.is<uint8_t>() || !frequency.is<uint32_t>()) {
    return nullptr;
  }

  return new I2CMasterDriver(t_name, bus_id, sda_pin, scl_pin, frequency);
}

}  // namespace HexFF
