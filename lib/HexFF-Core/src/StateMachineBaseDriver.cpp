#include "HexFF/StateMachineBaseDriver.h"


namespace HexFF {

auto BaseStateMachineModel::OnEvent::operator()(
    BaseStateMachineModel::State::IResumable&, const BaseStateMachineModel::Event::Resume&,
    BaseStateMachineModel::Context&) -> std::optional<std::variant<State::Error>> {
  return State::Error{};
}

//=======

auto BaseStateMachineModel::OnEvent::operator()(
    BaseStateMachineModel::State::IState&, const BaseStateMachineModel::Event::Resume&,
    BaseStateMachineModel::Context&) -> std::optional<std::variant<State::Error>> {
  return std::nullopt;
}

//=======

//auto BaseStateMachineModel::OnEvent::operator()(
//    BaseStateMachineModel::State::IResumable&, const BaseStateMachineModel::Event::Stop&,
//    BaseStateMachineModel::Context& t_context) -> std::optional<std::variant<State::Idle>> {
//  stop(t_context);
//  return State::Idle{};
//}

//=======

auto BaseStateMachineModel::OnEvent::operator()(
    BaseStateMachineModel::State::IState&, const BaseStateMachineModel::Event::Stop&,
    BaseStateMachineModel::Context&) -> std::optional<std::variant<State::Error>> {
  return std::nullopt;
}

//=======

//auto BaseStateMachineModel::OnEvent::operator()(
//    BaseStateMachineModel::State::IResumable&, const BaseStateMachineModel::Event::Reset&,
//    BaseStateMachineModel::Context& t_context) -> std::optional<std::variant<State::Idle>> {
//  stop(t_context);
//  return State::Idle{};
//}

//=======

auto BaseStateMachineModel::OnEvent::operator()(
    BaseStateMachineModel::State::IState&, const BaseStateMachineModel::Event::Reset&,
    BaseStateMachineModel::Context&) -> std::optional<std::variant<State::Idle>> {
  return State::Idle{};
}

}  // namespace HexFF
