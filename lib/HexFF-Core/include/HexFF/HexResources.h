#pragma once

#if !defined(HEXTRONICS_FIRMWARE_NAME) || !defined(HEXTRONICS_FIRMWARE_VERSION)
#error "Hextronics Framework requires the firmware name and version"
#endif

#if !defined(HEXTRONICS_BOARD_NAME) || !defined(HEXTRONICS_BOARD_VERSION)
#error "Hextronics Framework requires the board name and version"
#endif

#include "Driver.h"

#include <initializer_list>


namespace HexFF {

  class HexResources : public Driver {
  private:
    constexpr static const char *_NAME = "mother";
    unsigned long dotTime = 0;
    float hertz = 0;
    unsigned loopCount = 0;

  protected:
    bool customInit() override;

    bool customRefresh() override;

    void customReadStatus(JsonVariant doc, bool compressed) override;

    virtual void heartbeat(unsigned long currentMilis);

    virtual bool refreshSubscriberDrivers();

  public:
    HexResources(const HexResources &) = delete;
    HexResources &operator=(const HexResources &) = delete;
    ~HexResources() override = default;
    HexResources();
    virtual void initSerial();

    void readAllStatus(JsonVariant doc, bool compressed = true);

    template <typename T, typename... Args> T *createDriver(Args... args) {
      auto result = new T(args...);
      registerDriver({result});
      return result;
    }

    Driver *createDriver(DriverFactory &factory, const char *driverName = nullptr);

    void registerDriver(std::initializer_list<Driver *> drivers);
    void registerDriverForSetup(std::initializer_list<Driver *> drivers);
    void registerDriverAsSubscriber(std::initializer_list<Driver *> drivers);
    void registerDriverAsPublisher(std::initializer_list<Driver *> drivers);
    const char *getName() override;

    Driver *getDriver(const String &name);
    const Driver::DriverIndex &getDrivers() const;

    static void setHexBoard(HexResources *concrete);
    static HexResources *getHexBoard();

    JsonDocument getConfig();
    bool setConfig(const JsonDocument &config);

    constexpr static const char *DEFAULT_CONFIG_PATH = "/default_config.json";
    constexpr static const char *CONFIG_PATH = "/config.json";
  };
}  // namespace HexFF
