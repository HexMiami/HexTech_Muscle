#pragma once

#include "StateMachine.h"

namespace HexFF {

struct BaseStateMachineModel {
  struct State {
    struct IState {};

    struct IResumable : public IState {};

    struct Error : public IState {};
    struct Idle : public IState {};
  };

  struct Event {
    struct Resume {};
    struct Stop {};
    struct Reset {};
  };

  struct Context {
    bool isResumable;  // Current state is resumable
  };

  struct OnEvent {
    auto operator()(State::IResumable&, const Event::Resume&, Context&) -> std::optional<std::variant<State::Error>>;
    auto operator()(State::IState&, const Event::Resume&, Context&) -> std::optional<std::variant<State::Error>>;
    //    auto operator()(State::IResumable&, const Event::Stop&, Context&) -> std::optional<std::variant<State::Idle>>;
    auto operator()(State::IState&, const Event::Stop&, Context&) -> std::optional<std::variant<State::Error>>;
    //    auto operator()(State::IResumable&, const Event::Reset&, Context&) -> std::optional<std::variant<State::Idle>>;
    auto operator()(State::IState&, const Event::Reset&, Context&) -> std::optional<std::variant<State::Idle>>;

    template <typename TEvent>
    auto operator()(State::IState&, const TEvent&, Context&) -> std::optional<std::variant<State::Error>> {
      //      throw std::logic_error{ "Unsupported state transition" };
      return State::Error{};
    }

    //    static void stop(Context& t_context);
  };

  struct OnEntry {
    template <typename TState>
    void operator()(TState& t_state, Context& t_context) {
      std::visit(detail::overloaded{
                     [&](State::IResumable& _state) { t_context.isResumable = true; },
                     [&](State::IState& _state) { t_context.isResumable = false; },
                 },
                 t_state);
    }
  };

  struct OnExit {
    template <typename TState>
    void operator()(const TState&, Context&) {}
  };
};

}  // namespace HexFF
