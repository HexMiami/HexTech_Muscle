#pragma once

#include <optional>
#include <variant>

namespace HexFF {

namespace detail {

// helper type for the visitor #4
template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

// explicit deduction guide (not needed as of C++20)
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

}  // namespace detail

//=======

template <class... Args>
struct variant_cast_proxy {
  std::variant<Args...> v;

  template <class... ToArgs>
  operator std::variant<ToArgs...>() const {
    return std::visit([](auto&& arg) -> std::variant<ToArgs...> { return arg; }, v);
  }
};

template <class... Args>
auto variant_cast(const std::variant<Args...>& v) -> variant_cast_proxy<Args...> {
  return {v};
}

//=======

class NoAction {
 public:
  template <typename TState, typename TContext>
  void operator()(const TState&, const TContext&) {}
};

//=======

/**
 *
 * @tparam Context Context data to be use in each action
 * @tparam State Variant containing all the possible states
 * @tparam OnEvent Action to be executed by each transition. Returns new state or nullopt if the same state remains.
 * @tparam OnEntry Action to be executed when a new state is reached
 * @tparam OnExit Action to be executed when a state is left
 */
template <typename Context, typename State, typename OnEvent, typename OnEntry = NoAction, typename OnExit = NoAction>
class StateMachine {
 public:
  StateMachine(const Context& t_context, const State& t_init_state, const OnEvent& t_on_event,
               const OnEntry& t_on_entry = NoAction{}, const OnExit& t_on_exit = NoAction{})
      : context{t_context}, state_{t_init_state}, on_event_{t_on_event}, on_entry_{t_on_entry}, on_exit_{t_on_exit} {}

  void reset(const State& t_state) {
    state_ = t_state;
    on_entry_(state_, context);
  }

  template <typename Event>
  const State& process(const Event& t_event) {
    auto next_state = std::visit(
        [&](auto&& tt_state) -> std::optional<State> {
          if (auto next_state = on_event_(tt_state, t_event, context)) {
            return variant_cast(*next_state);
          }
          return std::nullopt;
        },
        state_);

    if (next_state) {
      on_exit_(state_, context);
      state_ = *next_state;
      on_entry_(state_, context);
    }

    return state_;
  }

  const State& getState() const { return state_; }

 protected:
  Context context;

 private:
  State state_;
  OnEvent on_event_;
  OnEntry on_entry_;
  OnExit on_exit_;
};

}  // namespace HexFF