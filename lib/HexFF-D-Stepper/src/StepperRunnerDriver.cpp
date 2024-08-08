#include "HexFF/StepperRunnerDriver.h"
#include "HexFF/HexResources.h"

namespace HexFF {

StepperRunnerDriver::StepperRunnerDriver(const std::vector<StepperDriver*>& _drivers)
    : Base(&StepperDriver::run, _drivers) {}

//=======

StepperRunnerDriver* FactoryStepperRunnerDriver::create(const char* t_name, const JsonVariant& t_settings,
                                                        const JsonVariant& t_extra_settings) {
  if (t_settings.isNull()) {
    return nullptr;
  }

  auto stepper_names = t_settings["stepper_drivers"].as<JsonArray>();
  if (stepper_names.isNull()) {
    return nullptr;
  }

  std::vector<StepperDriver*> steppers;
  steppers.reserve(stepper_names.size());

  for (JsonString stepper_name : stepper_names) {
    if (stepper_name == nullptr) {
      log_e("StepperRunner driver '%s' expected StepperDriver names.", stepper_name);
      return nullptr;
    }

    auto driver = reinterpret_cast<StepperDriver*>(HexResources::getHexBoard()->getDriver(stepper_name.c_str()));
    if (driver == nullptr) {
      log_e("Stepper driver '%s' does not exist.", stepper_name);
      return nullptr;
    }

    steppers.emplace_back(driver);
  }

  return new StepperRunnerDriver(steppers);
}

}  // namespace HexFF