#include "HexFF/PinOutputDriver.h"

#include <charconv>

#include "HexFF/utils.h"

namespace HexFF {

  PinOutputDriver::PinOutputDriver(const char *_name, uint8_t _pin, uint32_t _offValue,
                                   uint32_t _onValue)
      : name{_name}, pin{_pin}, offValue_{_offValue}, onValue_{_onValue} {}


  bool PinOutputDriver::customInit() {
    pinMode(pin, OUTPUT);
    write(offValue_);

    return true;
  }


  bool PinOutputDriver::customSet(const String &action) {
    std::string_view actionSV = action.c_str();
    const auto cmd = getToken(actionSV, "_");
    if (cmd == "on") {
      uint32_t value = onValue_;
      if (!actionSV.empty()) {
        const auto valueStr = getToken(actionSV, "_");
        std::from_chars(valueStr.begin(), valueStr.end(), value);
      }
      write(value);
    } else if (cmd == "off") {
      write(offValue_);
    } else {
      return false;
    }

    return true;
  }


  bool PinOutputDriver::customRefresh() {
    read();

    return false;
  }


  void PinOutputDriver::customReadStatus(JsonVariant doc, bool compressed) {
    if (compressed) {
      String data(static_cast<const char *>(doc["data"]));
      data += ':';
      data += value;
      doc["data"] = data;
    } else {
      doc["value"] = value;
    }
  }


  const char *PinOutputDriver::getName() { return name.c_str(); };


  void PinOutputDriver::write(uint32_t _value) { digitalWrite(pin, _value); }


  uint32_t PinOutputDriver::read() {
    value = digitalRead(pin);
    return value;
  }


  void PinOutputDriver::on() {
    write(onValue_);
  }


  void PinOutputDriver::off() {
    write(offValue_);
  }


  Driver* PinOutputDriverFactory::create(JsonVariant configSource, const char *name) {
    Driver *result = nullptr;
    if (name != nullptr && configSource.containsKey(name)) {
      auto driverConfig = configSource[name];
      if (driverConfig.isNull() || !driverConfig["pin"].is<uint8_t>()) {
        return nullptr;
      }

      auto pin = driverConfig["pin"].as<uint8_t>();
      uint32_t offValue = driverConfig["off_value"] | 0u;
      uint32_t onValue = driverConfig["on_value"] | 1u;
      result = new PinOutputDriver(name, pin, offValue, onValue);
    }
    return result;
  }


  PinOutputDriver* FactoryPinOutputDriver::create(const char* t_name, const JsonVariant& t_settings,
                                                  const JsonVariant& /*t_extra_settings*/) {
    if (t_settings.isNull()) {
      return nullptr;
    }

    auto pin = t_settings["pin"];

    if (!pin.is<uint8_t>()) {
      return nullptr;
    }

    uint32_t off_value = t_settings["off_value"] | 0u;
    uint32_t on_value = t_settings["on_value"] | 1u;

    return new PinOutputDriver(t_name, pin.as<uint8_t>(), off_value, on_value);
  }


  AnalogPinOutputDriver::AnalogPinOutputDriver(const char* _name, uint8_t _pin, uint32_t _offValue, uint32_t _onValue,
                                               uint32_t _frequency, uint8_t _resolution)
      : PinOutputDriver(_name, _pin, _offValue, _onValue), frequency_{_frequency}, resolution_{_resolution} {}


  bool AnalogPinOutputDriver::customInit() {
    pinMode(pin, OUTPUT);
    ledcAttach(pin, frequency_, resolution_);
    ledcWrite(pin, offValue_);

    return true;
  }


  bool AnalogPinOutputDriver::customSet(const String &action) {
    if (PinOutputDriver::customSet(action)) {
      return true;
    }

    std::string_view actionSV = action.c_str();
    const auto cmd = getToken(actionSV, "_");
    if (cmd == "duty") {
      const auto dutyStr = getToken(actionSV, "_");
      std::from_chars(dutyStr.begin(), dutyStr.end(), onValue_);
    } else if (cmd == "freq") {
      const auto freqStr = getToken(actionSV, "_");
      std::from_chars(freqStr.begin(), freqStr.end(), frequency_);
      ledcChangeFrequency(pin, frequency_, resolution_);
    } else {
      return false;
    }

    return true;
  }


  void AnalogPinOutputDriver::write(uint32_t _value) {
    ledcWrite(pin, _value);
  }


  uint32_t AnalogPinOutputDriver::read() {
    value = ledcRead(pin);
    return value;
  }


  Driver *AnalogPinOutputDriverFactory::create(JsonVariant configSource, const char *name) {
    Driver *result = nullptr;
    if (name != nullptr && configSource.containsKey(name)) {
      auto driverConfig = configSource[name];
      if (driverConfig.isNull()
          || !driverConfig["pin"].is<uint8_t>()
          || !driverConfig["channel"].is<uint8_t>()) {
        return nullptr;
      }

      auto pin = driverConfig["pin"].as<uint8_t>();
      auto channel = driverConfig["channel"].as<uint8_t>();
      uint32_t offValue = driverConfig["off_value"] | 0u;
      uint32_t onValue = driverConfig["on_value"] | 255u;
      uint32_t frequency = driverConfig["frequency"] | 1000u;
      uint8_t resolution = driverConfig["resolution"] | 8u;
      result = new AnalogPinOutputDriver(name, pin, offValue, onValue, frequency, resolution);
    }
    return result;
  }


  AnalogPinOutputDriver* FactoryPinOutputAnalogDriver::create(const char* t_name, const JsonVariant& t_settings,
                                                              const JsonVariant& /*t_extra_settings*/) {
    if (t_settings.isNull()) {
      return nullptr;
    }

    auto pin = t_settings["pin"];

    if (!pin.is<uint8_t>()) {
      return nullptr;
    }

    uint8_t off_value = t_settings["off_value"] | 0u;
    uint32_t on_value = t_settings["on_value"] | 255u;
    uint32_t frequency = t_settings["frequency"] | 1000u;
    uint8_t resolution = t_settings["resolution"] | 8u;

    return new AnalogPinOutputDriver(t_name, pin, off_value, on_value, frequency, resolution);
  }

  }  // namespace HexFF
