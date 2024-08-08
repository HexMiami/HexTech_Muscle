#pragma once

#include <WString.h>

#include "Driver.h"

namespace HexFF {

  class PinOutputDriver : public Driver {
  protected:
    const String name;
    const uint8_t pin;

    uint32_t offValue_;
    uint32_t onValue_;

    uint32_t value = LOW;

    bool customInit() override;
    bool customSet(const String &action) override;
    bool customRefresh() override;
    void customReadStatus(JsonVariant doc, bool compressed = true) override;

  public:
    explicit PinOutputDriver(const char *name, uint8_t pin, uint32_t offValue = LOW,
                             uint32_t onValue = HIGH);

    const char *getName() override;

    virtual void write(uint32_t value);
    virtual uint32_t read();
    virtual void on();
    virtual void off();

    const uint32_t &offValue = offValue_;
    const uint32_t &onValue = onValue_;
  };


  class PinOutputDriverFactory : public DriverFactory {
  public:
    Driver *create(JsonVariant configSource, const char *name) override;
  };


  class FactoryPinOutputDriver {
   public:
    static PinOutputDriver* create(const char* t_name, const JsonVariant& t_settings,
                                   const JsonVariant& t_extra_settings);

   private:
    static inline bool s_registered = FactoryDriver::add("POut", create);
  };


  class AnalogPinOutputDriver : public PinOutputDriver {
  protected:
    // PWM data
    uint32_t frequency_;
    uint8_t resolution_;

    bool customInit() override;
    bool customSet(const String &action) override;

  public:
    AnalogPinOutputDriver(const char *name, uint8_t pin, uint32_t offValue = LOW,
                          uint32_t onValue = 255, uint32_t frequency = 1000,
                          uint8_t resolution = 8);

    void write(uint32_t value) override;
    uint32_t read() override;

    const uint8_t &resolution = resolution_;
  };


  class AnalogPinOutputDriverFactory : public DriverFactory {
  public:
    Driver *create(JsonVariant configSource, const char *name) override;
  };

  //=======

  class FactoryPinOutputAnalogDriver {
  public:
   static AnalogPinOutputDriver* create(const char* t_name, const JsonVariant& t_settings,
                                 const JsonVariant& t_extra_settings);

  private:
   static inline bool s_registered = FactoryDriver::add("POutAnalog", create);
  };

}  // namespace HexFF