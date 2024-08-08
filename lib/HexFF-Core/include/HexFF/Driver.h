#pragma once

#include <ArduinoJson.h>
#include <WString.h>

#include <unordered_map>
#include <vector>

#include "factory.h"
#include "utils.h"

namespace HexFF {
  class Driver {
  protected:
    Driver() = default;
    virtual ~Driver() = default;
    Driver(Driver &&) = default;
    Driver &operator=(Driver &&) = default;
    Driver(const Driver &) = default;
    Driver &operator=(const Driver &) = default;

    /// @brief Default method to initialize the device
    ///        Child classes must overrride this method
    /// @return Return true on success, false in other case
    virtual bool customInit() { return true; };

    /// @brief Default method to change attributes of the device
    ///        Child classes must overrride this method
    /// @return Return true on success, false in other case
    virtual bool customSet(const String &action) { return true; };

    /// @brief Default method that run/update the current device status
    /// @return Return false to inform that device finished refreshing(running a command). Return
    /// true to inform that the device still need to maintain refreshing
    virtual bool customRefresh() { return false; };

    virtual void customReadStatus(JsonVariant doc, bool compressed = true) {
      // Default do not add other attributes
    };

  public:
    /// @brief Public interface to initialize a device.
    /// If the device is enabled Driver::enabled and is not initialized then it calls
    /// Driver::customInit method. If initialization is seccessfull(result of Driver::customInit)
    /// Driver::initialized attribute will be set to true.
    /// If initialization is not successful it will print an error and wait for board reset.
    /// @return Return true on success. Return false if the driver was already initialized
    virtual bool init();

    /// @brief Public interface to change attributes of the device
    /// If the device is enabled and initialized it calls Driver::customSet method.
    /// If device is not initialized it will print an error and wait for board reset.
    /// @return Return true on success, false in other case
    virtual bool set(const String &action);

    /// @brief Public interface that run/update the current device status
    /// If the device is enabled and initialized it calls Driver::customRefresh method.
    /// If device is not initialized it will print an error and wait for board reset.
    /// @return Return false to inform that device finished refreshing(running a command). Return
    /// true to inform that the device still need to maintain refreshing
    virtual bool refresh();

    /// @brief Public interface that serialize all attribute of the device
    /// It will serialize values for enabled and initialized attributes and
    /// call customReadStatus method for further readings.
    virtual void readStatus(JsonVariant doc, bool compressed = true);

    /// @brief Virtual interface to return the device name
    /// @return boolean
    virtual const char *getName() = 0;

    bool enabled = true;
    bool initialized = false;
    bool busy = false;

    using DriverIndex = std::unordered_map<const char *, Driver *, cstring_hash, cstring_equal_to>;
    using DriverCollection = std::vector<Driver *>;
    static DriverCollection driversForSetup;
    static DriverCollection publisherDrivers;
    static DriverCollection subscriberDrivers;
    static DriverIndex driverIndex;
  };

  class DriverFactory {
  public:
    virtual Driver *create(JsonVariant configSource, const char *name) = 0;
  };

  template <typename T> DriverFactory &getDriverFactory() {
    static T instance;
    return instance;
  };

  //=======

  class FactoryDriver
      : public Factory<Driver, std::function<Driver*(const char*, const JsonVariant&, const JsonVariant&)>> {};

}  // namespace HexFF
