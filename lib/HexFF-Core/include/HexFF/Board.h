#pragma once

#include "HexResources.h"

namespace HexFF {

class Board: public HexResources {
 public:
  Board();

  const char* getName() override;

  bool load(const char* t_filename);

 protected:
  bool refreshSubscriberDrivers() override;

  const char* notification_driver = "led.yl";
  const char* notification_busy_actions[2] = {"off", "on"};


};

}
