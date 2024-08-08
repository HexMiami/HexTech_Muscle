#pragma once

#include "HexFF/Driver.h"
#include "HexFF/PinInputDriver.h"

#include "dumbledore_stepper.h"

namespace HexFF {

class StepperDriver : public Driver, public DumbledoreStepper {
 private:
  uint8_t pinCS;
  float pspeed;
  float yspeed;
  int pacc;
  uint16_t rms;
  uint16_t microStepVal;
  uint16_t stallGuardThreshold;
  TCA9535* stepperSelector;
  TCA9535* extSwitches;
  PinInputDriver* zeroLimit;

  bool customInit() override;
  bool customSet(const String& action) override;
  bool customRefresh() override;
  void customReadStatus(JsonVariant doc, bool compressed = true) override;

 public:
  StepperDriver(const char* name, TCA9535* dirSelectExpander, uint8_t dir, uint8_t step, uint8_t cs,
                SPIClass* spiInterface, TCA9535* stepperSelector, TCA9535* extSwitches, PinInputDriver* btn = nullptr,
                const float pspeed = P_SPEED, const float yspeed = Y_SPEED, const int pacc = PACC, uint16_t rms = RMS,
                uint16_t microStepVal = MICRO_STEP_VAL, uint16_t stallGuardThreshold = STALL_GUARD_THRESHOLD,
                uint8_t limitButtonPin = 255);

  const char* getName() override;

  constexpr static const char* CATEGORY_NAME = "stepper";
  constexpr static const float P_SPEED = 2000;
  constexpr static const float Y_SPEED = 2000;
  constexpr static const int PACC = 1800;
  constexpr static const uint16_t RMS = 1500;
  constexpr static const uint16_t MICRO_STEP_VAL = 2;
  constexpr static const uint16_t STALL_GUARD_THRESHOLD = 6;
};

//=======

class FactoryStepperDriver {
 public:
  static StepperDriver* create(const char* t_name, const JsonVariant& t_settings, const JsonVariant& t_extra_settings);

 private:
  static inline bool s_registered = FactoryDriver::add("StepperMotor", create);
};

}  // namespace HexFF