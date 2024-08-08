#include "HexFF/Driver.h"

#include <Arduino.h>
#include <esp32-hal-log.h>

namespace HexFF {

  Driver::DriverCollection Driver::driversForSetup;
  Driver::DriverCollection Driver::publisherDrivers;
  Driver::DriverCollection Driver::subscriberDrivers;
  Driver::DriverIndex Driver::driverIndex;

  [[noreturn]] void waitForReset() {
    do {
      delay(1);
    } while (true);
  }
  bool Driver::init() {
    if (enabled) {
      if (!initialized) {
        initialized = this->customInit();
        if (!initialized) {
          log_e("%s: initialization failed", this->getName());
          waitForReset();
        }
      } else {
        log_w("%s: Driver already initialized", this->getName());
        return false;
      }
    }
    return initialized;
  }

  bool Driver::set(const String &action) {
    if (enabled) {
      if (!initialized) {
        log_e("%s: Driver is not initialized", this->getName());
        waitForReset();
      }
      /// Before set configurations make sure busy flag is enabled
      busy = true;
      if (action.equals("print_serial")) {
        JsonDocument doc;
        this->readStatus(doc);
        Serial.println("StartReport");
        serializeJson(doc, Serial);
        Serial.print("\nEndReport\n");
        Serial.println("Done!");
        return true;
      }
      bool result = customSet(action);

      /// After finish the configuration we put back busy flag.
      /// Eventually refresh method properly re-update if needed.
      /// @see Driver::refresh()
      busy = false;

      return result;
    }
    return false;
  };

  bool Driver::refresh() {
    bool result = false;
    if (enabled) {
      if (!initialized) {
        log_e("%s: Driver is not initialized", this->getName());
        waitForReset();
      }
      result = customRefresh();
    }
    busy = result;
    return busy;
  }

  void Driver::readStatus(JsonVariant doc, bool compressed) {
    auto childDoc = doc[this->getName()].to<JsonObject>();
    if (compressed) {
      String data;
      data += enabled;
      data += ':';
      data += initialized;
      data += ':';
      data += busy;
      childDoc["data"] = data;
    } else {
      childDoc["enabled"] = enabled;
      childDoc["initialized"] = initialized;
      childDoc["busy"] = busy;
    }
    customReadStatus(childDoc, compressed);
  };
};  // namespace HexFF
