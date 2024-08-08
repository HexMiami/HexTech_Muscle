#include "HexFF/Board.h"

#include <Arduino.h>
#include <LittleFS.h>

namespace HexFF {

Board::Board() : HexResources() {}

//=======

const char* Board::getName() {
  return "hmr.0";
}

//=======

bool Board::load(const char* t_filename) {
  if (!HexFF::isFilesystemInitialized()) {
    log_e("Filesystem not initialized");
    return false;
  }

  JsonDocument config;
  {
    auto file = LittleFS.open(t_filename, "r");
    if (auto error = deserializeJson(config, file); error != DeserializationError::Ok) {
      log_e("%s: deserializeJson() failed with code %s", t_filename, error.c_str());
      return false;
    }
    file.close();
  }

  auto extra_settings = config["extra_settings"].as<JsonObject>();
  auto drivers = config["drivers"].as<JsonArray>();
  for (JsonObject driver_config : drivers) {
    auto type_name = driver_config["type"].as<const char*>();
    if (type_name == nullptr || !(driver_config["enable"] | true)) {
      continue;
    }

    auto name = driver_config["name"].as<const char*>();
    auto settings = driver_config["settings"];

    auto p_driver = FactoryDriver::create(type_name, name, settings, extra_settings);

    if (p_driver == nullptr) {
      log_e("%s: driver configuration does not exist or is broken.", name);
      return false;
    }

    registerDriver({p_driver});

    if (driver_config["init"] | true) {
      registerDriverForSetup({p_driver});
      p_driver->init();
    }

    if (driver_config["subscribe"] | true) {
      registerDriverAsSubscriber({p_driver});
    }

    if (driver_config["publish"]) {
      registerDriverAsPublisher({p_driver});
    }
  }

  return true;
}

//=======

bool Board::refreshSubscriberDrivers() {
  auto result = HexResources::refreshSubscriberDrivers();

  if (result != busy) {
    driverIndex.at(notification_driver)->set(notification_busy_actions[result]);
  }

  return result;
}

}  // namespace HexFF
