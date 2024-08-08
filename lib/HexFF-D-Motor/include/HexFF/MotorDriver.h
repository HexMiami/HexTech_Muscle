#pragma once

#include "HexFF/Driver.h"
#include "HexFF/PinOutputDriver.h"
#include "HexFF/StateMachineBaseDriver.h"

namespace HexFF {

struct MotorSMModel {
  struct Context : public BaseStateMachineModel::Context {
    const String name;

    AnalogPinOutputDriver* pin_pwm;
    PinOutputDriver* pin_dir;

    uint32_t max_speed;
    uint32_t accel_time;
    uint32_t decel_time;

    bool reversed_dir;

    bool dir;
    double accel;
    double decel;
    uint32_t speed;

    uint32_t move_time;
    uint32_t start_time;
  };

  struct State : public BaseStateMachineModel::State {
    struct Accelerating : public IResumable {};

    struct Cruising : public IResumable {};

    struct Decelerating : public IResumable {};

    using Any = std::variant<Error, Idle, Accelerating, Cruising, Decelerating>;
  };

  struct Event : public BaseStateMachineModel::Event {
    struct Move {
      int32_t time;
    };
  };

  struct OnEvent : public BaseStateMachineModel::OnEvent {
    using BaseStateMachineModel::OnEvent::operator();

    auto operator()(State::Idle&, const Event::Move&, Context&) -> std::optional<std::variant<State::Accelerating>>;
    auto operator()(State::Accelerating&, const Event::Resume&,
                    Context&) -> std::optional<std::variant<State::Cruising>>;
    auto operator()(State::Cruising&, const Event::Resume&,
                    Context&) -> std::optional<std::variant<State::Decelerating>>;
    auto operator()(State::Decelerating&, const Event::Resume&, Context&) -> std::optional<std::variant<State::Idle>>;

    auto operator()(State::IResumable&, const Event::Move&, Context&) -> std::optional<std::variant<State::Idle>>;
    auto operator()(State::IResumable&, const Event::Stop&, Context&) -> std::optional<std::variant<State::Idle>>;

    static bool computeDir(Context&, int32_t t_time);
    static void updateSpeed(Context&, double t_accel, uint32_t t_speed_time);
    static void stop(Context& t_context);
  };

  struct OnEntry : public BaseStateMachineModel::OnEntry {
    template <typename TState>
    void operator()(TState& t_state, Context& t_context) {
      BaseStateMachineModel::OnEntry::operator()(t_state, t_context);

      Serial.printf("[%s : %lu] ", t_context.name.c_str(), millis());
      std::visit(detail::overloaded{
                     [&](State::Idle&) { Serial.println("--> Idle"); },
                     [&](State::Accelerating&) { Serial.println("--> Accelerating"); },
                     [&](State::Cruising&) { Serial.println("--> Cruising"); },
                     [&](State::Decelerating&) { Serial.println("--> Decelerating"); },
                     [&](State::Error&) { Serial.println("--> Error"); },
                 },
                 t_state);
    }
  };
};

//=======

using MotorStateMachine = StateMachine<MotorSMModel::Context, MotorSMModel::State::Any, MotorSMModel::OnEvent,
                                       MotorSMModel::OnEntry, BaseStateMachineModel::OnExit>;

//=======

class MotorDriver : public Driver, public MotorStateMachine {
 public:
  MotorDriver(const char* t_name, AnalogPinOutputDriver* t_pin_pwm, PinOutputDriver* t_pin_dir,
              uint32_t t_max_speed_pct = 100, uint32_t t_accel_time = 10, uint32_t t_decel_time = 10,
              bool t_reversed_dir = false);

  const char* getName() override;

 protected:
  bool customInit() override;
  bool customSet(const String& action) override;
  bool customRefresh() override;

  static uint32_t computeSpeed(uint8_t t_speed_pct, uint8_t t_resolution);
  void updateAccel();
};

//=======

class FactoryDCMotorDriver {
 public:
  static MotorDriver* create(const char* t_name, const JsonVariant& t_settings, const JsonVariant& t_extra_settings);

 private:
  static inline bool s_registered = FactoryDriver::add("DCMotor", create);
};

}  // namespace HexFF