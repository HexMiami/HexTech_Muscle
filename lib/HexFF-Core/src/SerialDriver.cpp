#include "HexFF/SerialDriver.h"

#include <Arduino.h>

#include "HexFF/MessageDispatcher.h"

namespace HexFF {

  ///   TODO: If Global Object of Properties changes, then refresh the configuration also.
  bool SerialDriver::customRefresh() {
    if (Serial.available()) {
      String message = Serial.readStringUntil('\n');
      if (MessageDispatcher::getInstance().push(message)) {
        log_i("Got it!");
        return true;
      }
    }
    return false;
  }
  const char *SerialDriver::getName() { return _NAME; }

}  // namespace HexFF