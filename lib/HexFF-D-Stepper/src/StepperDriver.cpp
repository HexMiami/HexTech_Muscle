#include "HexFF/StepperDriver.h"

#include "HexFF/HexResources.h"
#include "HexFF/SPIDriver.h"
#include "HexFF/TCA9535Driver.h"

namespace HexFF {

StepperDriver::StepperDriver(const char* name, TCA9535* dirSelectExpander, uint8_t dir, uint8_t step, uint8_t cs,
                             SPIClass* spiInterface, TCA9535* _stepperSelector, TCA9535* _extSwitches,
                             PinInputDriver* _btn, const float _pspeed, const float _yspeed, const int _pacc,
                             uint16_t _rms, uint16_t _microStepVal, uint16_t _stallGuardThreshold,
                             uint8_t limitButtonPin)
    : DumbledoreStepper(dirSelectExpander, dir, step, cs, spiInterface, _stepperSelector),
      pinCS(cs),
      stepperSelector(_stepperSelector),
      extSwitches(_extSwitches),
      pspeed(_pspeed),
      yspeed(_yspeed),
      pacc(_pacc),
      rms(_rms),
      microStepVal(_microStepVal),
      stallGuardThreshold(_stallGuardThreshold),
      zeroLimit(_btn) {
  stepName(name, "");
}

bool StepperDriver::customInit() {
  /// TODO: Improve DumbledoreStepper class constructor and setup.
  /// Constructor is for allocate the
  /// resources, setup just for configuration
  stepperSetup(rms, microStepVal, stallGuardThreshold, yspeed, pacc, yspeed, extSwitches);

//  // stepperSelector.writePin(static_cast<uint8_t>(getCSIndex()), HIGH);
//  stepperSelector.writePin(static_cast<uint8_t>(PIN::STEPPER_CS_0), HIGH);
//  stepperSelector.writePin(static_cast<uint8_t>(PIN::STEPPER_CS_1), HIGH);
//  stepperSelector.writePin(static_cast<uint8_t>(PIN::STEPPER_CS_2), HIGH);
//  stepperSelector.writePin(static_cast<uint8_t>(PIN::STEPPER_CS_3), HIGH);
  stepperSelector->writePin(static_cast<uint8_t>(pinCS), HIGH);
  return true;
}


bool StepperDriver::customSet(const String& action) {
  String splitResult[4];
  long int parseItems = split(action, splitResult, 3, "_");
  if (parseItems >= 1) {
    int speedArg = 0;
    if (splitResult[0].compareTo("moveUpTo") == 0) {
      speedArg = 1000000;
    } else if (splitResult[0].compareTo("moveDownTo") == 0) {
      speedArg = -1000000;
    }
    if (speedArg != 0 && parseItems == 2) {
      auto switchLimit = HexResources::getHexBoard()->getDriver(splitResult[1]);
      if (switchLimit != nullptr) {
        // Send move long enough. The switch limit will be supervised to stop the motor
        zeroLimit = reinterpret_cast<PinInputDriver*>(switchLimit);
        splitResult[0] = "move";
        splitResult[1] = String(speedArg);
        splitResult[2] = "1";
        parseItems = 3;
      } else {
        log_e("Driver '%s' is not created", splitResult[1].c_str());
        return false;
      }
    }
    return this->command(splitResult, parseItems);
  }
  return false;
}


bool StepperDriver::customRefresh() {
  busy = isBusy();
  if (zeroLimit != nullptr && zeroLimit->getValue() != HIGH && busy) {
    this->stop();
    this->stopMotor();
    this->endZero();
    zeroLimit = nullptr;
  }
  if (watchStall()) {
    return false;
  }
  return isBusy();
}


void StepperDriver::customReadStatus(JsonVariant doc, bool compressed) {
  if (compressed) {
    String value(static_cast<const char*>(doc["data"]));
    value += ':';
    value += currentPosition();  // 3
    value += ':';
    value += targetPosition();  // 4
    value += ':';
    value += zeroFail;  // 5
    value += ':';
    value += actionTime;  // 6
    value += ':';
    value += hitPin;  // 7
    value += ':';
    value += zeroPin;  // 8
    value += ':';
    value += zeroError;  // 9
    value += ':';
    value += zeroTime;  // 10
    value += ':';
    value += _stalled;  // 11
    value += ':';
    value += baseStallThreshold;  // 12
    value += ':';
    value += _stallGuardThreshold;  // 13
    value += ':';
    value += distWent;  // 14
    value += ':';
    value += isHome();  // 15
    value += ':';
    value += idleLock;  // 16
    value += ':';
    value += _speedVal;  // 17
    value += ':';
    value += unplugged;  // 18
    value += ':';
    value += _accelVal;  // 19
    value += ':';
    value += _microStepVal;  // 20
    value += ':';
    value += _rms;  // 21
    value += ':';
    value += _zeroDirection;  // 22
    value += ':';
    value += driveDir;  // 23
    value += ':';
    value += sgUnlock;  // 24
    value += ':';
    value += sgValue;  // 25
    value += ':';
    value += sgAverage;  // 26
    value += ':';
    value += checkDone();  // 27
    // value += ':';
    // value += driver.TCOOLTHRS();
    // value += ':';
    // value += driver.TSTEP();

    doc["data"] = value;
  } else {
    doc["iPos"] = this->currentPosition();
    doc["tPos"] = this->targetPosition();
    doc["zeroFail"] = this->zeroFail;
    doc["actionTime"] = this->actionTime;
    doc["hitPin"] = hitPin;
    doc["zeroPin"] = zeroPin;
    doc["zeroError"] = zeroError;
    doc["zeroTime"] = zeroTime;
    doc["stalled"] = _stalled;
    doc["baseStallThreshold"] = baseStallThreshold;
    doc["stallGuardThreshold"] = _stallGuardThreshold;
    doc["distWent"] = distWent;
    doc["isHome"] = isHome();
    doc["idleLock"] = idleLock;
    doc["speedVal"] = _speedVal;
    doc["unplugged"] = unplugged;
    doc["accelVal"] = _accelVal;
    doc["microStepVal"] = _microStepVal;
    doc["rms"] = _rms;
    doc["zeroDirection"] = _zeroDirection;
    doc["driveDir"] = driveDir;
    doc["sgUnlock"] = sgUnlock;
    doc["sgValue"] = sgValue;
    doc["sgAverage"] = sgAverage;
    doc["stopped"] = checkDone();
    // doc["TCOOLTHRS"] = driver.TCOOLTHRS();
    // doc["TSTEP"] = driver.TSTEP();
    // doc["DIAG0"] = driver.diag0_stall();
    // doc["DIAG1"] = driver.diag1_stall();
  }
}


const char* StepperDriver::getName() {
  return getStepperName();
}

//=======

StepperDriver* FactoryStepperDriver::create(const char* t_name, const JsonVariant& t_settings,
                                            const JsonVariant& t_extra_settings) {
  if (t_settings.isNull()) {
    return nullptr;
  }

  auto spi_driver_name = t_settings["spi_driver"].as<const char*>();
  auto tca9535_driver_name = t_settings["tca9535_driver"].as<const char*>();
  auto pin_dir = t_settings["pin_dir"];
  auto pin_step = t_settings["pin_step"];
  auto pin_cs = t_settings["pin_cs"];
  //  auto frequency = t_settings["frequency"];
  if (spi_driver_name == nullptr || tca9535_driver_name == nullptr || !pin_dir.is<uint8_t>() ||
      !pin_step.is<uint8_t>() || !pin_cs.is<uint8_t>()) {
    return nullptr;
  }

  auto spi_driver = reinterpret_cast<SPIDriver*>(HexResources::getHexBoard()->getDriver(spi_driver_name));
  if (spi_driver == nullptr) {
    log_e("SPI driver '%s' does not exist.", tca9535_driver_name);
    return nullptr;
  }

  auto tca9535_driver = reinterpret_cast<TCA9535Driver*>(HexResources::getHexBoard()->getDriver(tca9535_driver_name));
  if (tca9535_driver == nullptr) {
    log_e("TCA9535 driver '%s' does not exist.", tca9535_driver_name);
    return nullptr;
  }

  return new StepperDriver(t_name, tca9535_driver, pin_dir, pin_step, pin_cs, spi_driver, tca9535_driver, nullptr);
}

}  // namespace HexFF
