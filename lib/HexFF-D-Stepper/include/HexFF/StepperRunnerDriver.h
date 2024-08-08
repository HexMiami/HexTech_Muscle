#pragma once

#include <functional>

#include "HexFF/MemberFnRefreshDriver.hpp"
#include "HexFF/StepperDriver.h"

namespace HexFF {

class StepperRunnerDriver : public MemberFnRefreshDriver<StepperDriver> {
 public:
  using Base = MemberFnRefreshDriver<StepperDriver>;

  explicit StepperRunnerDriver(const std::vector<StepperDriver*>& _drivers);

  const char* getName() override { return "stepper-runner"; };
};

//=======

class FactoryStepperRunnerDriver {
 public:
  static StepperRunnerDriver* create(const char* t_name, const JsonVariant& t_settings,
                                     const JsonVariant& t_extra_settings);

 private:
  static inline bool s_registered = FactoryDriver::add("StepperMotorRunner", create);
};

}  // namespace HexFF