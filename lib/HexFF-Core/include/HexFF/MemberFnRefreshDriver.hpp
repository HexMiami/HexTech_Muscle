#pragma once

#include <functional>
#include <initializer_list>

#include "Driver.h"

namespace HexFF {
  template <typename _Class> class MemberFnRefreshDriver : public Driver {
  private:
    using TMemFn = boolean (_Class::*)();
    std::_Mem_fn<TMemFn> memberCall;
    std::vector<_Class *> drivers;

  protected:
    bool customRefresh() override {
      bool result = false;
      for (auto *driver : this->drivers) {
        bool driverResult = memberCall(*driver);
        result = result || driverResult;
      }
      return result;
    }

  public:
    explicit MemberFnRefreshDriver(TMemFn fp, const std::vector<_Class *>& _drivers)
        : memberCall(std::mem_fn(fp)), drivers(_drivers) {
      this->initialized = true;
    }

    const char *getName() override { return _NAME; }

    constexpr static const char *_NAME = "memfn::runner";
  };
}  // namespace HexFF