#include <Arduino.h>

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
  vTaskDelete(nullptr);
}

//=======

__attribute__((noreturn)) void refresh(void*) {
  log_i("Refresh task started.");
  try {
    for (;;) {
      // Actions
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
  log_i("Stepper runner task started.");
  for (;;) {
    // Actions
  }
}
