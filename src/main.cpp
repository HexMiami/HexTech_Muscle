#include <Arduino.h>

#include "HexFF/Board.h"

// Drivers required by firmware
#include "HexFF/I2CMasterDriver.h"
#include "HexFF/PinFlasherDriver.h"
#include "HexFF/SPIDriver.h"
#include "HexFF/VoltageDriver.h"
#include "HexFF/PinOutputDriver.h"

#include "HexFF/TCA9535Driver.h"

#include "HexFF/StepperDriver.h"
#include "HexFF/StepperRunnerDriver.h"

#include "HexFF/MotorDriver.h"

HexFF::Board board;

void init(void*);
__attribute__((noreturn)) void refresh(void*);
__attribute__((noreturn)) void runSteppers(void*);

// put function declarations here:
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {}

  xTaskCreatePinnedToCore(init,             /* Task function. */
                          "hm_init",        /* name of task. */
                          1024 * 16,        /* Stack size of task */
                          nullptr,          /* parameter of the task */
                          tskIDLE_PRIORITY, /* priority of the task */
                          nullptr,          /* Task handle to keep track of created task */
                          0);               /* pin task to core 0 */

  xTaskCreatePinnedToCore(refresh,          /* Task function. */
                          "hm_refresh",     /* name of task. */
                          1024 * 64,        /* Stack size of task */
                          nullptr,          /* parameter of the task */
                          tskIDLE_PRIORITY, /* priority of the task */
                          nullptr,          /* Task handle to keep track of created task */
                          0);               /* pin task to core 1 */

  xTaskCreatePinnedToCore(runSteppers,       /* Task function. */
                          "hm_run_steppers", /* name of task. */
                          1024 * 8,          /* Stack size of task */
                          nullptr,           /* parameter of the task */
                          1,                 /* priority of the task */
                          nullptr,           /* Task handle to keep track of created task */
                          1);                /* pin task to core 1 */
}

//=======

void loop() {
  // Remove the task of the loop. Cores will be busy on other task. See setup()
  vTaskDelete(nullptr);
}

//=======

void init(void*) {
  if (!board.load("/default_config.json") || !board.init()) {
    log_e("Firmware loading/initializing failed!");
    esp_restart();
  }

  vTaskDelete(nullptr);
}

//=======

__attribute__((noreturn)) void refresh(void*) {
  do {
    delay(300);
  } while (!board.initialized);

  log_i("Refresh task started.");
  try {
    for (;;) {
      // Actions
      board.refresh();
    }
  } catch (std::exception& e) {
    log_e("%s", e.what());
  } catch (...) {
    log_e("Unknown exception");
  }
  esp_restart();
}

//=======

__attribute__((noreturn)) void runSteppers(void*) {
  do {
    delay(300);
  } while (!board.initialized);

  auto stepperRunner = HexFF::HexResources::driverIndex.at("stepper-runner");
  log_i("Stepper runner task started.");
  for (;;) {
    // Actions
    stepperRunner->refresh();
  }
}
