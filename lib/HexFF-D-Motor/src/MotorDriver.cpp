#include "HexFF/MotorDriver.h"
#include <charconv>
#include "HexFF/HexResources.h"

namespace HexFF {

auto MotorSMModel::OnEvent::operator()(State::Idle&, const Event::Move& t_event,
                                       Context& t_context) -> std::optional<std::variant<State::Accelerating>> {
  if (t_event.time == 0) {
    return std::nullopt;
  }

  // Set direction: clockwise or counter-clockwise
  //  t_context.pin_dir->write(!((t_event.time > 0) ^ !t_context.reversed_dir));
  t_context.dir = computeDir(t_context, t_event.time);
  t_context.pin_dir->write(t_context.dir);

  t_context.move_time = std::abs(t_event.time);
  t_context.start_time = millis();

  return State::Accelerating{};
}

//=======

auto MotorSMModel::OnEvent::operator()(State::Accelerating&, const Event::Resume&,
                                       Context& t_context) -> std::optional<std::variant<State::Cruising>> {
  auto moving_time = millis() - t_context.start_time;
  updateSpeed(t_context, t_context.accel, moving_time);

  if (t_context.speed >= t_context.max_speed) {
    return State::Cruising{};
  }

  return std::nullopt;
}

//=======

auto MotorSMModel::OnEvent::operator()(State::Cruising&, const Event::Resume&,
                                       Context& t_context) -> std::optional<std::variant<State::Decelerating>> {
  auto moving_time = millis() - t_context.start_time;

  if (moving_time + t_context.decel_time >= t_context.move_time) {
    return State::Decelerating{};
  }

  return std::nullopt;
}

//=======

auto MotorSMModel::OnEvent::operator()(State::Decelerating&, const Event::Resume&,
                                       Context& t_context) -> std::optional<std::variant<State::Idle>> {
  auto moving_time = millis() - t_context.start_time;
  auto decel_time = t_context.move_time - std::min(moving_time, t_context.move_time);
  updateSpeed(t_context, t_context.decel, decel_time);

  if (t_context.speed == 0) {
    return State::Idle{};
  }

  return std::nullopt;
}

//=======

auto MotorSMModel::OnEvent::operator()(State::IResumable&, const Event::Move& t_event,
                                       MotorSMModel::Context& t_context) -> std::optional<std::variant<State::Idle>> {
  if (t_event.time == 0 || t_context.dir != computeDir(t_context, t_event.time)) {
    return std::nullopt;
  }

  t_context.move_time += std::abs(t_event.time);

  return std::nullopt;
}

//=======

auto MotorSMModel::OnEvent::operator()(State::IResumable&, const Event::Stop&,
                                       Context& t_context) -> std::optional<std::variant<State::Idle>> {
  stop(t_context);
  return State::Idle{};
}

//=======

bool MotorSMModel::OnEvent::computeDir(Context& t_context, int32_t t_time) {
  return !((t_time > 0) ^ !t_context.reversed_dir);
}

//=======

void MotorSMModel::OnEvent::updateSpeed(Context& t_context, double t_accel, uint32_t t_speed_time) {
  t_context.speed = std::min(uint32_t(t_accel * t_speed_time), t_context.max_speed);
  t_context.pin_pwm->write(t_context.speed);
}

//=======

void MotorSMModel::OnEvent::stop(Context& t_context) {
  t_context.pin_pwm->off();
}

//=======

MotorDriver::MotorDriver(const char* t_name, AnalogPinOutputDriver* t_pin_pwm, PinOutputDriver* t_pin_dir,
                         uint32_t t_max_speed_pct, uint32_t t_accel_time, uint32_t t_decel_time, bool t_reversed_dir)
    : MotorStateMachine(MotorSMModel::Context{false, t_name, t_pin_pwm, t_pin_dir,
                                              computeSpeed(t_max_speed_pct, t_pin_pwm->resolution), t_accel_time,
                                              t_decel_time, t_reversed_dir},
                        MotorSMModel::State::Idle{},  //
                        MotorSMModel::OnEvent{},      //
                        MotorSMModel::OnEntry{},      //
                        BaseStateMachineModel::OnExit{}) {}

//=======

const char* MotorDriver::getName() {
  return context.name.c_str();
}

//=======

bool MotorDriver::customInit() {
  if (!(context.pin_pwm->initialized || context.pin_pwm->init())
      || !(context.pin_dir->initialized || context.pin_dir->init())) {
    return false;
  }

  updateAccel();

  process(MotorSMModel::Event::Reset{});
  return true;
}

//=======

bool MotorDriver::customSet(const String& action) {
  std::string_view action_sv = action.c_str();
  const auto cmd = getToken(action_sv, "_");
  if (cmd == "move") {
    const auto timeStr = getToken(action_sv, "_");
    long time = 0;
    std::from_chars(timeStr.begin(), timeStr.end(), time);
    process(MotorSMModel::Event::Move{time});
  } else if (cmd == "stop") {
    process(MotorSMModel::Event::Stop{});
  } else if (cmd == "speed") {
    const auto speedStr = getToken(action_sv, "_");
    uint8_t speed = 0;
    std::from_chars(speedStr.begin(), speedStr.end(), speed);
    context.max_speed = computeSpeed(speed, context.pin_pwm->resolution);
    updateAccel();
  } else {
    return false;
  }
  return true;
}

//=======

bool MotorDriver::customRefresh() {
  process(MotorSMModel::Event::Resume{});
  return context.isResumable;
}

//=======

uint32_t MotorDriver::computeSpeed(uint8_t t_speed_pct, uint8_t t_resolution) {
  auto max_speed = (1 << t_resolution) - 1;
  return map(t_speed_pct, 0, 100, 0, max_speed);
}

//=======

void MotorDriver::updateAccel() {
  context.accel = double(context.max_speed) / context.accel_time;
  context.decel = double(context.max_speed) / context.decel_time;
}

//=======

MotorDriver* FactoryDCMotorDriver::create(const char* t_name, const JsonVariant& t_settings, const JsonVariant&) {
  if (t_settings.isNull()) {
    return nullptr;
  }

  auto pin_pwm_driver_name = t_settings["pin_pwm_driver"].as<const char*>();
  auto pin_dir_driver_name = t_settings["pin_dir_driver"].as<const char*>();
  if (pin_pwm_driver_name == nullptr || pin_dir_driver_name == nullptr) {
    return nullptr;
  }

  auto pin_pwm_driver =
      reinterpret_cast<AnalogPinOutputDriver*>(HexResources::getHexBoard()->getDriver(pin_pwm_driver_name));
  if (pin_pwm_driver == nullptr) {
    log_e("Analog Output Pin (PWM) driver '%s' does not exist.", pin_pwm_driver_name);
    return nullptr;
  }
  auto pin_dir_driver = reinterpret_cast<PinOutputDriver*>(HexResources::getHexBoard()->getDriver(pin_dir_driver_name));
  if (pin_dir_driver == nullptr) {
    log_e("Output Pin (Dir) driver '%s' does not exist.", pin_dir_driver_name);
    return nullptr;
  }

  uint8_t max_speed = t_settings["max_speed"] | 100u;
  uint8_t accel_time = t_settings["accel_time"] | 10u;
  uint8_t decel_time = t_settings["decel_time"] | 10u;
  bool reversed_dir = t_settings["reversed_dir"] | false;

  return new MotorDriver(t_name, pin_pwm_driver, pin_dir_driver, max_speed, accel_time, decel_time, reversed_dir);
}

}  // namespace HexFF