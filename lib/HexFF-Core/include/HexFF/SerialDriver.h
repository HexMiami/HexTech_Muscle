#pragma once

#include "Driver.h"

namespace HexFF {
  class SerialDriver : public Driver {
  protected:
    bool customRefresh() override;

  public:

    const char *getName() override;

    const char *_NAME = "serial";
  };
}  // namespace HexFF