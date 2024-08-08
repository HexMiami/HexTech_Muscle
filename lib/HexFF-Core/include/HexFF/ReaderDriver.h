#pragma once

#include <esp32-hal-gpio.h>

#include "HexFF/Driver.h"


namespace HexFF {
template <typename T>
class ReaderDriver : public Driver {
 protected:
  void customReadStatus(JsonVariant doc, bool compressed = true) override {
    if (compressed) {
      String data(static_cast<const char*>(doc["data"]));
      data += ':';
      data += (*this)();
      doc["data"] = data;
    } else {
      doc["value"] = (*this)();
    }
  }

 public:
  virtual T operator()() const = 0;

  T getValue() const { return (*this)(); }
};


template <typename T>
class BasePinReaderDriver : public ReaderDriver<T> {
 protected:
  const uint8_t pin_;

  bool customInit() override {
    pinMode(pin_, INPUT);
    return ReaderDriver<T>::customInit();
  }

 public:
  explicit BasePinReaderDriver(uint8_t _pin) : pin_{_pin} {}
};

}  // namespace HexFF