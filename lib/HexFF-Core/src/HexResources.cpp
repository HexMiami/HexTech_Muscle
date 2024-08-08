#include "HexFF/HexResources.h"

#include <Arduino.h>

#include <ArduinoJson.h>

#include <LittleFS.h>

#include <SafeStringReader.h>

#include "HexFF/MessageDispatcher.h"
#include "HexFF/utils.h"
#include "HexFF/SerialDriver.h"

namespace HexFF {

  constexpr static const char COMMAND_LIST_DELIMITER = ';';
  constexpr static const char COMMAND_ARGS_DELIMITER = '_';

  HexResources *HexBoard = nullptr;

  Driver *serialDriver = new SerialDriver();

  JsonDocument *configJSON = nullptr;

  /*
   *  Adds first parsing logic
   */
  bool parseAndRun(SafeString &cmd);

  bool parseAndRunList(char *cmdList);

  bool preActions(SafeString &cmd);

  void resetConfigJSONObject();

  HexResources::HexResources() {
    setHexBoard(this);
    registerDriver({this});
  }

  void HexResources::setHexBoard(HexResources *concrete) { HexBoard = concrete; }
  HexResources *HexResources::getHexBoard() { return HexBoard; }

  void HexResources::initSerial() {
    Serial.setDebugOutput(true);
    if (serialDriver != nullptr) {
      serialDriver->init();
    }
  }

  bool HexResources::customInit() {
    initSerial();
    for (auto driver : Driver::driversForSetup) {
      if (!driver->initialized) {
        driver->init();
      }
    }
    return true;
  }

  bool HexResources::refreshSubscriberDrivers() {
    bool result = false;
    for (auto *driver : subscriberDrivers) {
      bool driverResult = driver->refresh();
      result = result || driverResult;
    }
    return result;
  }

  void HexResources::heartbeat(unsigned long currentMilis) {
    if ((currentMilis - dotTime) > 5000) {
      hertz = (static_cast<float>(loopCount) / 5);
      dotTime = currentMilis;
      loopCount = 0;
    }
    loopCount++;
  }

  bool HexResources::customRefresh() {
    static unsigned long readTime = 0;
    auto currentMilis = millis();
    heartbeat(currentMilis);
    try {
      bool startingCmd = false;
      if ((currentMilis - readTime) > 400) {
        if (serialDriver != nullptr) {
          serialDriver->refresh();
        }
        MessageDispatcher &messageDispatcher = MessageDispatcher::getInstance();
        if (MessageDispatcher::Message hmMsg;
            busy ? messageDispatcher.popPriority(hmMsg) : messageDispatcher.pop(hmMsg)) {
          // Process the message here
          log_i("Command List: %s", hmMsg.data());
          if (!(startingCmd = parseAndRunList(hmMsg.data()))) {
            log_e("Bad Command!");
            Serial.println();
          }
        }
        readTime = millis();
      }

      auto result = this->refreshSubscriberDrivers();
      if ((busy || startingCmd) && !result) {
        // FIXME If an error occurs in internal driver (pad, gripper, ...), we log "Bad Command!" and "Done!"
        log_i("Done!");
      }

      return result;

    } catch (...) {
      log_e("Fatal Error.");
    }
    return false;
  }

  void HexResources::customReadStatus(JsonVariant doc, bool compressed) {
    doc["firmwareName"] = HEXTRONICS_FIRMWARE_NAME;
    doc["firmwareVersion"] = HEXTRONICS_FIRMWARE_VERSION;
    doc["boardName"] = HEXTRONICS_BOARD_NAME;
    doc["boardVersion"] = HEXTRONICS_BOARD_VERSION;
    doc["version"] = HEXTRONICS_FIRMWARE_VERSION;
    if (compressed) {
      String value = static_cast<const char *>(doc["data"]);
      value += ':';
      value += hertz;
      value += ':';
      value += ESP.getFreeHeap();
      value += ':';
      value += uxTaskGetStackHighWaterMark(nullptr);
      doc["data"] = value;
    } else {
      doc["hertz"] = hertz;
      doc["freeHeap"] = ESP.getFreeHeap();
      doc["freeStack"] = uxTaskGetStackHighWaterMark(nullptr);
    }

    for (const auto &[name, driver] : driverIndex) {
      if (driver == this) {
        continue;
      }
      driver->readStatus(doc, compressed);
    }
  }

  void HexResources::readAllStatus(JsonVariant doc, bool compressed) {
    Driver::readStatus(doc, compressed);
    for (const auto &[name, driver] : driverIndex) {
      driver->readStatus(doc, compressed);
    }
  }

  const char *HexResources::getName() { return _NAME; }

  Driver *HexResources::getDriver(const String &name) {
    if (auto it = Driver::driverIndex.find(name.c_str()); it != Driver::driverIndex.end()) {
      return it->second;
    }
    return nullptr;
  }

  const Driver::DriverIndex &HexResources::getDrivers() const { return Driver::driverIndex; }

  void HexResources::registerDriver(std::initializer_list<Driver *> drivers) {
    for (auto driver : drivers) {
      Driver::driverIndex.emplace(driver->getName(), driver);
    }
  }

  void HexResources::registerDriverForSetup(std::initializer_list<Driver *> drivers) {
    for (auto driver : drivers) {
      Driver::driversForSetup.emplace_back(driver);
    }
  }

  void HexResources::registerDriverAsSubscriber(std::initializer_list<Driver *> drivers) {
    for (auto driver : drivers) {
      Driver::subscriberDrivers.emplace_back(driver);
    }
  }
  void HexResources::registerDriverAsPublisher(std::initializer_list<Driver *> drivers) {
    for (auto driver : drivers) {
      Driver::publisherDrivers.emplace_back(driver);
    }
  }

  Driver *HexResources::createDriver(DriverFactory &factory, const char *driverName) {
    if (isFilesystemInitialized()) {
      // First time assign memory on configuration object
      if (configJSON == nullptr) {
        resetConfigJSONObject();
        // Select file name
        const char *configPath = HexResources::DEFAULT_CONFIG_PATH;
        if (!LittleFS.exists(HexResources::DEFAULT_CONFIG_PATH)) {
          String msg(HexResources::DEFAULT_CONFIG_PATH);
          msg += ": Does not exist";
          reboot(msg.c_str());
        }
        if (LittleFS.exists(CONFIG_PATH)) {
          configPath = HexResources::CONFIG_PATH;
        }

        // Read configuration Object
        File file = LittleFS.open(configPath, "r");
        deserializeJson(*configJSON, file);
        file.close();
      }

      // Create and return the resource requested
      auto result = factory.create(*configJSON, driverName);
      if (result != nullptr) {
        registerDriver({result});
        return result;
      }
    }
    String msg(driverName != nullptr ? driverName : "nullptr");
    msg += ": Driver configuration does not exist or is broken";
    reboot(msg.c_str());
    return nullptr;
  }


  JsonDocument HexResources::getConfig() {
    return configJSON != nullptr ? *configJSON : JsonDocument();
  }


  bool HexResources::setConfig(const JsonDocument& _configJSON) {
    File file = LittleFS.open(HexResources::CONFIG_PATH, FILE_WRITE);

    if (!file) {
      return false;
    }

    merge(*configJSON, _configJSON);
    serializeJsonPretty(*configJSON, file);
    file.close();

    return true;
  }


  /*
   *  Adds first parsing logic
   */
  bool parseAndRun(SafeString &cmd) {
    createSafeString(driverName, 16);
    auto cmdLength = cmd.length();
    bool haveToken = cmd.firstToken(driverName, COMMAND_ARGS_DELIMITER);
    if (haveToken) {
      const char *action = "";
      if (cmdLength > driverName.length()) {
        action = cmd.c_str() + 1;
      }
      auto driver = HexResources::getHexBoard()->getDriver(driverName.c_str());
      if (driver != nullptr) {
        return driver->set(action);
      }
    }
    return false;
  }

  bool parseAndRunList(char *cmdList) {
    createSafeStringFromCharPtr(inputLine, cmdList);  // enough space for the input text
    createSafeString(cmd,
                     257);  // for the field strings. Should have capacity > largest field length
    // firstToken/nextToken consumes input so this only runs once
    bool haveToken = inputLine.firstToken(cmd, COMMAND_LIST_DELIMITER);
    // bool haveToken = inputLine.nextToken(field, ',',true,true,true); // firstToken == nextToken
    // with the last (optional) arg set to true
    while (haveToken) {
      cmd.trim();
      bool executed = preActions(cmd);
      if (!executed && !parseAndRun(cmd)) {
        return false;
      }

      haveToken = inputLine.nextToken(
          cmd, COMMAND_LIST_DELIMITER);  // skips empty fields by default,  use
                                         // optional arg (true) to return empty fields
    }
    return true;
  }

  bool preActions(SafeString &cmd) {
    if (cmd.equals("reboot")) {
      esp_restart();
      return true;
    }
    if (cmd.equals("localReport")) {
      HexResources::getHexBoard()->set("print_serial");
      return true;
    }
    else if (cmd.equals("fullReport")) {
      HexResources::getHexBoard()->getDriver("master")->set("print_serial");
      return true;
    }
    else if (cmd.equals("print_free")) {
      Serial.print("Free Heap: ");
      Serial.print(ESP.getFreeHeap());
      Serial.println(" bytes");

      // Print free stack memory
      Serial.print("Free Stack: ");
      Serial.print(uxTaskGetStackHighWaterMark(nullptr));
      Serial.println(" bytes");
      return true;
    }
//    if (cmd.equals("localReport")) {
//      JsonDocument doc;
//      HexResources::getHexBoard()->readAllStatus(doc);
//      Serial.println("StartReport");
//      serializeJsonPretty(doc, Serial);
//      Serial.print("\nEndReport\n");
//      Serial.println("Done!");
//      return true;
//    }
    return false;
  }

  void resetConfigJSONObject() {
    if (configJSON != nullptr) {
      delete configJSON;
    }
    configJSON = new JsonDocument();
  }

}  // namespace HexFF