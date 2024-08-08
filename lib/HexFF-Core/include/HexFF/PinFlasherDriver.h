#pragma once

#include <WString.h>

#include <PinFlasher.h>

#include "Driver.h"

namespace HexFF {
  class PinFlasherDriver : public Driver, public PinFlasher {
    unsigned long rate;
    const String name;

  protected:
    bool customInit() override;
    bool customSet(const String &status) override;
    bool customRefresh() override;

  public:
    PinFlasherDriver(const char *name, uint8_t pin, unsigned long rate = FLASH_RATE);
    const char *getName() override;
    constexpr static const int FLASH_RATE = 200;
  };


  class FactoryPinFlasherDriver {
   public:
    static PinFlasherDriver* create(const char* t_name, const JsonVariant& t_settings,
                                    const JsonVariant& t_extra_settings);

   private:
    static inline bool s_registered = FactoryDriver::add("PFlasher", create);
  };

}  // namespace HexFF