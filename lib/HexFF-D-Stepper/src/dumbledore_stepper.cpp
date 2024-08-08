
#include "HexFF/dumbledore_stepper.h"
#include <set>


// dummy functions to call the proper constructor for AccelStepper
void DumbledoreStepper::fwd() {}
void DumbledoreStepper::back() {}

void DumbledoreStepper::step0(long phase) {
  (void)(phase);  // Unused
  // set STEP Low, will remain low for as long as we do our stuff, should be enough to meet timing
  // without additional delays
  digitalWrite(_stepPin, LOW);
  digitalWrite(_stepPin, HIGH);
}

DumbledoreStepper::DumbledoreStepper(TCA9535* dirSelectExpander, uint8_t dirSelectPin,
                                     uint8_t stepPin, uint8_t CSIndex, SPIClass *spi_interface,
                                     TCA9535* stepperDrivers)
    : driver(CSIndex, spi_interface, *stepperDrivers),  // pass the CSIndex to the TMC constructor
      AccelStepper(&DumbledoreStepper::fwd,
                   &DumbledoreStepper::back)  // pass our dummy fwd and back functions to the
                                              // constructor of our base class. This will cause the
                                              // base class to call Step0, which we are overriding
{
  _dirSelectExpander = dirSelectExpander;  // save pointer to dir select expander
  _dirSelectPin = dirSelectPin;
  _stepPin = stepPin;
  _CSIndex = CSIndex;
  stallClock = 0;
  incrementerVal = 1;
  _zeroDirection = 1;
  driveDir = 1;
  stallCycleDelay = 400;  // NOT USED
  stallClockDelay = 32;
  plugClockDelay = 100;

  _toff = 4;
  idleLock = 0;
  _stalled = false;
  startPos = 0;
  // zeroStartingPos = 0;
  zeroError = 0;
  startTime = 0;
  hitPin = false;
  stillTrying = false;
  zeroStartTime = 0;
  timeout = 20000;
  zeroFail = false;
  zeroTime = 0;
  lastStallTime = millis();
  lastPlugTime = millis();
  stallIncrementer = 0;
  sgValueMax = 150;
  sgValueMin = 100;
  timeoutProtection = false;
  OL_3 = 0;
  OL_2 = 0;
  OL_1 = 0;
  unplugged = false;
  actionTime = 0;
  firstZero = true;
  useAverageStall = false;
}

void DumbledoreStepper::begin() {
  // Initialize the TMC2160 driver
  driver.begin();
  // define the Step pin as output
  pinMode(_stepPin, OUTPUT);
  _dirSelectExpander->setPinMode(_dirSelectPin, OUTPUT);
  stealth(true);
  this->stop();
  this->stopMotor();
}

/*************Nick's Code******************/
bool DumbledoreStepper::command(const String *const args, int n) {
  const String first = args[0];
  if (first == "stop") {
    this->stop();
    this->stopMotor();
    return true;
  }

  if (first == "idle") {
    this->driver.toff(0);
    return true;
  }

  if (first == "setZero") {
    endZero();
    return true;
  }

  second = args[1].toInt();

  if (first == "go") {
    auto distance = second - this->currentPosition();
    setDir(distance >= 0 ? 1 : -1);
    this->startMotor();
    int third = args[2].toInt();
    this->stealth((bool)third);
    this->moveTo(second);
    return true;
    this->timeoutProtection = false;
  }
  if (first == "move") {
    setDir(second >= 0 ? 1 : -1);
    this->startMotor();
    int third = args[2].toInt();
    this->stealth((bool)third);
    this->moveTo(std::abs(second) * driveDir + this->currentPosition());
    return true;
  }
  if (first == "moveStealth") {
    setDir(second >= 0 ? 1 : -1);
    this->startMotor();
    this->stealth(true);
    this->moveTo(std::abs(second) * driveDir + this->currentPosition());
    return true;
  }
  if (first == "speed") {
    _speedVal = second;
    setSpeed((float)_speedVal);
    setMaxSpeed((float)_speedVal);
    accelTime = this->maxSpeed() / _accelVal;
    return true;
  }
  if (first == "stall") {
    // Check the valid range for the stallguard
    if (second >= -64 && second <= 63) {
      _stallGuardThreshold = (int8_t)second;
      this->driver.stallGuardConf(_stallGuardThreshold);
      baseStallThreshold = _stallGuardThreshold;
      return true;
    } else {
      return false;
    }
  }
  if (first == "toff") {
    _toff = static_cast<uint8_t>(second);
    this->driver.toff(_toff);
    return true;
  }
  if (first == "zeroDir") {
    setDir(static_cast<short>(second));
    _zeroDirection = std::abs(second) * driveDir;
    this->zeroSpeed = zeroSpeed * static_cast<float>(_zeroDirection);
    return true;
  }

  if (first == "accel") {
    _accelVal = second;
    setAcceleration(_accelVal);
    accelTime = this->maxSpeed() / _accelVal;
    return true;
  }
  if (first == "hyst") {
    // hysteresis_start(
    this->driver.hysteresis_start(second);  // Hysteresis of the motor. lower = quieter and lower
                                            // microstep accuracy. higher = accurate and loud
    int third = args[2].toInt();
    this->driver.hysteresis_end(third);
    return true;
  }
  if (first == "filt") {
    this->driver.filt_isense(second);
    return true;
  }
  if (first == "microsteps") {
    _microStepVal = second;
    this->driver.microsteps(second);
    return true;
  }
  if (first == "stealth") {
    this->stealth(!second);
    // this->driver.en_pwm_mode(second);       // Toggle stealthChop on TMC2130/2160/5130/5160
    // this->driver.pwm_autoscale(second);     // Needed for stealthChop
    return true;
  }

  if (first == "sgValueMax") {
    sgValueMax = uint16_t(second);
    return true;
  }
  if (first == "sgValueMin") {
    sgValueMin = uint16_t(second);
    return true;
  }
  if (first == "enableStallStop") {
    enableStallStop = (second != 0);
    return true;
  }
  if (first == "zeroSpeed") {
    zeroSpeed = second;
    return true;
  }
  if (first == "stallDelay") {
    stallDelay = second;
    return true;
  }
  if (first == "rms") {
    _rms = second;
    this->driver.rms_current(_rms);
    return true;
  }
  if (first == "driveDir") {
    setDir(static_cast<short>(second));
    return true;
  }
  if (first == "stallCycle") {
    this->stallClockDelay = second;
    this->stallCycleDelay = second;
    return true;
  }
  if (first == "tbl") {
    this->driver.tbl(second);
    return true;
  }
  if (first == "chm") {
    this->driver.chm(second);
    return true;
  }

  if (first == "idleLock") {
    idleLock = second;
    return true;
  }
  if (first == "sgUnlock") {
    sgUnlock = second;
    return true;
  }
  if (first == "timeout") {
    timeout = second * 1000;
    return true;
  }
  if (first == "semin") {
    this->driver.semin(second);
    return true;
  }
  if (first == "semax") {
    this->driver.semax(second);
    return true;
  }
  if (first == "seup") {
    this->driver.seup(second);
    return true;
  }
  if (first == "sedn") {
    this->driver.sedn(second);
    return true;
  }
  if (first == "coolThrsh") {
    this->driver.TCOOLTHRS(second);
    return true;
  }
  if (first == "sfilt") {
    this->driver.sfilt(second);
    return true;
  }
  if (first == "useAverageStall") {
    if (second == 0) {
      useAverageStall = false;
    }
    if (second == 1) {
      useAverageStall = true;
    }
    return true;
  }
  return false;
}

///////*** end of command handler****/////
//**start of stepper setup****///
// stepper0.stepperSetup(1400, 64, sgtVal, yspeed, pacc, yspeed/4)
void DumbledoreStepper::stepperSetup(uint16_t rms, uint16_t microStepVal,
                                     uint16_t stallGuardThreshold, float speedVal, float accVal,
                                     float zeroSpeedVal, TCA9535* buttonComms) {
  //_buttonComms = &buttonComms;
  _rms = rms;
  _speedVal = speedVal;
  _stallGuardThreshold = stallGuardThreshold;
  _lastStallThreshold = _stallGuardThreshold;
  baseStallThreshold = _stallGuardThreshold;
  _microStepVal = microStepVal;
  this->driver.rms_current(rms);
  this->driver.microsteps(microStepVal);
  this->driver.stallGuardConf(stallGuardThreshold);
  this->setMaxSpeed((float)speedVal);
  _accelVal = accVal;
  this->setAcceleration(accVal);
  zeroSpeed = zeroSpeedVal;
  endEvent = false;
  this->begin();
  accelTime = speedVal / _accelVal;
  sgUnlock = 1;
  elapsed = 0;

  distWent = 0;
}
///****start of motor specific functions*****///
void DumbledoreStepper::stealth(bool action) {
  // Serial.println("Setting Stealth ="+String(action));
  // this->driver.chm(!action); //Martin Commented out
  sgON = !action;
  // Serial.println("Stall Guard active?: " + String(sgON));
  this->driver.en_pwm_mode(action);    // Toggle stealthChop on TMC2130/2160/5130/5160
  this->driver.pwm_autoscale(action);  // Needed for stealthChop

  // some defaults to tune spreadCycle
  // this->driver.chm(0);//force spreadCycle - 0 = spreadcycle, 1= classic chopper //Martin
  // Commented out sgUnlock = 0; //sgUnlock = 0 means keep SG value locked; sgUnlock = 1 means try
  // to auto-adapt SG this->driver.TCOOLTHRS(3800);//enable stallguard at relatively low speed
  // this->driver.TPWMTHRS(50);//enable stealthchop below this speed
}

long DumbledoreStepper::getPosition(void) { return this->currentPosition(); }
bool DumbledoreStepper::isBusy(void) { return this->distanceToGo() != 0; }
// bool DumbledoreStepper::isHome(void) {
//   return !_buttonComms->readPin(zeroPin);
//   //  return digitalRead(this->limit_pin);
// }
// long DumbledoreStepper::zeroError(void) {
//   return (second + lastPos);
// }
// void DumbledoreStepper::storePos(void) {
// }

long DumbledoreStepper::rememberPos(void) { return lastPos; }
void DumbledoreStepper::setZeroSpeed(float inputSpeed, short multiplier) {
  zeroSpeed = inputSpeed / multiplier;
}

bool DumbledoreStepper::checkDone(void) {
  bool isDone = (!(this->isBusy()) || endEvent);
  if (isDone) {
    if (stillTrying == true) {
      // this->actionTime = millis()- this->startTime;
      this->stillTrying = false;
      this->stopMotor();
    }
  } else {
    stillTrying = true;
  }
  return isDone;
}

void DumbledoreStepper::idle(void) {
  if (idleLock == 0) {
    this->driver.toff(0);
  } else if ((millis() - startTime) > 2000000) {
    this->driver.toff(0);
  }
}

void DumbledoreStepper::startMotor(void) {
  unplugged = false;
  this->driver.toff(_toff);
  this->endEvent = false;
  startTime = millis();
  startPos = this->currentPosition();
}
/**
 *
 */
void DumbledoreStepper::stopMotor(void) {
  // adding stallStuff So it doesn't break 4/24/23
  this->distWent = this->currentPosition() - startPos;  // Moved here to fix reporting on 7/20/23
  _stallGuardThreshold = baseStallThreshold;
  this->driver.stallGuardConf(baseStallThreshold);
  this->stealth(true);                                // End of stall stuff
  this->setCurrentPosition(this->currentPosition());  // this forces internal speed to 0, and set
                                                      // target as current position
  this->endEvent = true;
  actionTime = millis() - startTime;
  this->idle();
}

bool DumbledoreStepper::watchStall() {
  auto updateSgValueReadings = [this]() {
    if (sgValueReadingsIndex >= 0) {
      this->sgValueReadingsTotal -= this->sgValueReadings[sgValueReadingsIndex];
      this->sgValueReadings[sgValueReadingsIndex] = sgValue;
      this->sgValueReadingsTotal += this->sgValueReadings[sgValueReadingsIndex];
      ++sgValueReadingsIndex;
      if (sgValueReadingsIndex >= numSgValueReadings) {
        sgValueReadingsIndex = 0;
      }
    } else {
      sgValueReadingsIndex = 0;
      std::fill(this->sgValueReadings.begin(), this->sgValueReadings.end(), sgValue);
      this->sgValueReadingsTotal = sgValue * numSgValueReadings;
      ++sgValueReadingsIndex;
    }
    sgAverage = sgValueReadingsTotal / numSgValueReadings;
  };
  if (millis() - lastStallTime > stallClockDelay) {
    lastStallTime = millis();
    sgValue = driver.sg_result();
    updateSgValueReadings();
    if (sgON) {
      lastStallTime = millis();
      _stalled = false;
      if (this->isRunning()) {
        // this->run();//call this right before and right after the bus access to minimize jitter
        // caused by the relatively long bus access
        uint32_t drv_status = driver.DRV_STATUS();  // this reads all status words in a single
                                                    // operation. It is faster than reading
        //, then OLA, then OLB, etc
        // this->run();//call this right before and right after the bus access to minimize jitter
        // caused by the relatively long bus access get the fields of interest from the drv_satus
        // bytes

        bool ola = drv_status & 0x20000000;  // bit 29
        bool olb = drv_status & 0x40000000;  // bit 30 //
        // bool stallFlag = drv_status & (1 << 24);  // bit 24
        //  bool stallFlag = driver.read(26);

        OL_3 = OL_2;
        OL_2 = OL_1;
        OL_1 = ola || olb;

        elapsed = millis() - startTime;
        // Serial.println(String("10: ") + elapsed + " | " + (accelTime * 1000 + stallDelay) + " | " + accelTime + " | " + stallDelay);
        Serial.println(String("20: ") + elapsed + " | " + sgValue + " | " + sgValueMax + " | " + sgAverage + " | " + _stallGuardThreshold + " | " + stallIncrementer);
        if ((elapsed > accelTime * 1000 + stallDelay)) {
          ++stallIncrementer;
          if (sgUnlock == 1 && stallIncrementer >= 3) {
            // Serial.println(String("20: ") + elapsed + " | " + sgValue + " | " + sgValueMax + " | " + sgAverage + " | " + _stallGuardThreshold + " | " + stallIncrementer);
            if (sgAverage > sgValueMax && _stallGuardThreshold > -60) {  //
              _stallGuardThreshold = _stallGuardThreshold - 1;
              Serial.println(
                  stepperName + " new stall threshold: "
                  + String(_stallGuardThreshold));  // + ", Check time: " + String(stallCheckTime)
              this->driver.stallGuardConf(_stallGuardThreshold);
              stallIncrementer = 0;
            }
            // Serial.println(String(sgValue) + "," + String(sgAverage));
            // Serial.println(String("XXX ") + String(sgValue) + "," + String(sgAverage));
          }
          // else {
          //   // Serial.println(stepperName+ " stall: " + String(sgValue) + " ola = " +
          //   String(ola)
          //   +
          //   // " olb = "+ String(olb)); // + ", Check time: " + String(stallCheckTime)
          //    Serial.println(String(sgValue) + "," + String(sgAverage) + "," +
          //    String(stallFlag));
          // }

          // Detecting a stall with one or multiple values, added on 4/24/23
          bool condition = false;
          if (enableStallStop) {
            if (useAverageStall == true && sgAverage < 10) {
              condition = true;
            } else if (useAverageStall == false && sgValue == 0) {
              condition = true;
            }
          }

          // useAverageStall
          if (condition == true) {
            Serial.println(String("STALL 0 ") + useAverageStall + " | " + sgValue + " | " + sgAverage);
            _stalled = true;   // whether we stalled
            stallZero = true;  // whether or not we zero'd using a stall
            // Stop the stepper and reset all the internal variables to ensure it remains stopped
            // this->stop();//not really necessary, setting the target position as the current
            // position and forcing the speed to zero is enough
            long to_go = distanceToGo();  // save distance to go
            Serial.println(String("STALL 1 ") + to_go + " | " + targetPosition() + " | " + currentPosition());
            float currSpeed = speed();
            // log_i("Stall detected: %d", this->distanceToGo());
            this->stopMotor();
            Serial.println(String("STALL 2 ") + distanceToGo() + " | " + targetPosition() + " | " + currentPosition());
            incrementerVal = 1;
            // UNCOMMENT
            // Serial.println(stepperName + " stallGuarded at: " + String(sgValue) + ", After " +
            // String(elapsed) + " Millis" + ", Threshold: " + String(_stallGuardThreshold) + " with
            // "+ to_go + " uSteps to go at speed of "+String(currSpeed)+" uSteps / second" );
            _lastStallThreshold = _stallGuardThreshold;
            _stallGuardThreshold = baseStallThreshold;
            this->driver.stallGuardConf(
                baseStallThreshold);  // MAJOR BUG MISSING THIS LINE 12/20/22
            // Serial.println("StallThreshold reset to " + (String)baseStallThreshold); //UNCOMMENT
            this->stealth(true);  // commented out by MGL on 11/29/22
            Serial.println(String("STALL 3 ") + distanceToGo() + " | " + targetPosition() + " | " + currentPosition());
            return true;
          }
        }
      }
    }
  }

  if (!sgON && stillTrying && (millis() - lastPlugTime > plugClockDelay)) {
    uint32_t drv_status
        = driver.DRV_STATUS();  // this reads all status words in a single operation. It is faster
                                // than reading SGValue, then OLA, then OLB, etc
    bool ola = drv_status & 0x20000000;  // bit 29
    bool olb = drv_status & 0x40000000;  // bit 30 //
    OL_3 = true;
    OL_2 = OL_1;
    OL_1 = ola || olb;
    lastPlugTime = millis();
  }

  if (OL_3 == OL_2 && OL_2 == OL_1 && OL_1 == 1) {
    Serial.println("Error:004:" + stepperName + ": Became disconnected.");
    OL_3 = false;
    OL_2 = false;
    OL_1 = false;
    unplugged = true;
  }
  return false;
}

void DumbledoreStepper::stallDetectAverage(bool cond) { useAverageStall = cond; }

void DumbledoreStepper::zeroInit(void) {
  this->driver.toff(_toff);
  zeroFail = false;
  hitPin = false;
  this->endEvent = false;
  startTime = millis();
  zeroStartTime = millis();
  stallZero = false;
  this->stallZero = false;
  timeoutProtection = false;
}

bool DumbledoreStepper::setZero(void) {
  lastPos = this->currentPosition();
  this->setCurrentPosition(0);  // this sets the current position, target position and speed to 0.
                                // @see AccelStepper::setCurrentPosition

  return true;
}
void DumbledoreStepper::endZero(void) {
  this->stopMotor();
  if (firstZero == false) {
    zeroError = this->currentPosition();  // Double Check this!
  } else if (firstZero == true) {
    zeroError = 0;
    firstZero = false;
  }
  this->setZero();
  this->stealth(true);
  zeroTime = millis() - zeroStartTime;
  setMaxSpeed((float)_speedVal);
}
void DumbledoreStepper::handleDone(void) {}
void DumbledoreStepper::setTimeoutProtection(void) { this->timeoutProtection = true; }
void DumbledoreStepper::setZeroPin(int pinNumber) { zeroPin = pinNumber; }
bool DumbledoreStepper::getSGstatus(void) { return sgON; }
int DumbledoreStepper::getTOFF(void) { return this->_toff; }
void DumbledoreStepper::setDir(short dirSet) {
  /// Change the direction if we need to
  if (driveDir >= 0 && dirSet < 0 || driveDir < 0 && dirSet >= 0) {
    // stop the motor before change directions
    stop();
    stopMotor();

    driveDir = dirSet >= 0 ? 1 : -1;
    if (driveDir > 0)
      _dirSelectExpander->clrPin(_dirSelectPin);
    else {
      _dirSelectExpander->setPin(_dirSelectPin);
    }
  }
}

void DumbledoreStepper::stepName(const String &_stepperName, const String &_stepperNum) {
  this->stepperNum = _stepperNum;
  this->stepperName = _stepperName;
}

bool DumbledoreStepper::isHome() { return this->currentPosition() == 0; }

///////////////////////////////////////////////////

/********************************************************************************************
   Functions for class DumbledoreTMC - to initialize the TMC 2160 stepper driver
   and to access it through the port expander
*/

DumbledoreTMC::DumbledoreTMC(uint8_t csIndex, SPIClass *spi_interface, TCA9535 &stepperSelector)
    : TMC2160Stepper(-1, 0.05),
      _csIndex(csIndex),
      SPI_interface(spi_interface),
      selector(stepperSelector)  // call constructor with -1 for CS pin and 0.05 ohms Rs
{}

void DumbledoreTMC::begin() {
  selector.setPinMode(_csIndex, OUTPUT);
  deactivateCSPins(selector, _csIndex);
  // Some initial default values defined here. Can be changed later
  this->toff(4);   // Enables driver in software
  this->hstrt(5);  // Hysteresis of the motor. hstrt changes the current forced into the stepper.
                   // hend changes the hysteresis end
  // 0&12 or 5&5
  this->hend(5);
  this->chm(0);  // Chop mode to be the spreadcycle (0 = true)
  this->filt_isense(0);
  this->tbl(3);  // 2-3 // 3 works the best
  this->sfilt(1);

  // this->TCOOLTHRS(0xFFFFF);
  // this->diag0_stall(true);
  // this->diag1_stall(true);
  // this->diag0_int_pushpull(1);
  // this->diag1_pushpull(1);
}

// This function activates the CS line of one particular driver
// It does this by callin
// selectStepperCS(x), where x is a number from 0 to 10
void DumbledoreTMC::switchCSpin(bool state) {
  /// TODO Check if it is required to put all CS-steppers to HIGH
  /// TODO: Make TMC independently. Like other Driver for example
  if (state) {
//    selector.writePin(static_cast<uint8_t>(HM::PIN::STEPPER_CS_0), HIGH);
//    selector.writePin(static_cast<uint8_t>(HM::PIN::STEPPER_CS_1), HIGH);
//    selector.writePin(static_cast<uint8_t>(HM::PIN::STEPPER_CS_2), HIGH);
//    selector.writePin(static_cast<uint8_t>(HM::PIN::STEPPER_CS_3), HIGH);
    deactivateCSPins(selector, _csIndex);
  } else {
    selector.writePin(_csIndex, LOW);
  }
//  selector.writePin(_csIndex, state);
}

////////////////////////////////////////////////////////
// These three below should not be called,
// but we are overriding them just in case
void DumbledoreTMC::beginTransaction() {}
void DumbledoreTMC::endTransaction() {}
uint8_t DumbledoreTMC::transfer(const uint8_t data) { return SPI_interface->transfer(data); }
/////////////////////////////////////////////////////

// Redefine the Write function for the stepper driver to
// handle our own special mode of driving CS and to use our
// own SPI driver
#define TMC_WRITE 0x80  // 10000000
void DumbledoreTMC::write(uint8_t addressByte, uint32_t config) {
  addressByte |= TMC_WRITE;
  switchCSpin(HIGH);
  SPI_interface->beginTransaction(
      SPISettings(2UL * 1000 * 1000, MSBFIRST, SPI_MODE3));  // mode 3, 2MHz (max allowed is 4MHz)

  switchCSpin(LOW);
  status_response = SPI_interface->transfer(addressByte);
  SPI_interface->transfer(config >> 24);  // right shift operator (>>)
  SPI_interface->transfer(config >> 16);
  SPI_interface->transfer(config >> 8);
  SPI_interface->transfer(config);
  switchCSpin(HIGH);

  SPI_interface->endTransaction();  // release settings for other routines
}

void DumbledoreTMC::stallGuardConf(int8_t B) {
  // 0xFFFFF0 = 11111111 11111111 11110000 = -16
  // needs to be bytes 16 to 22
  // uint8_t B = 0xFFFFF0;
  this->sgt(B);
}

uint16_t DumbledoreTMC::pollStallGuard(void) {
  uint16_t sgValue = this->sg_result();
  return sgValue;
}
uint32_t DumbledoreTMC::read(uint8_t addressByte) {
  uint32_t out = 0UL;
  switchCSpin(HIGH);
  SPI_interface->beginTransaction(
      SPISettings(2UL * 1000 * 1000, MSBFIRST, SPI_MODE3));  // mode 3, 2MHz (max allowed is 4MHz)

  // send the address to read followed by 4 dummy bytes
  switchCSpin(LOW);
  SPI_interface->transfer(addressByte);
  SPI_interface->transfer(0x00);
  SPI_interface->transfer(0x00);
  SPI_interface->transfer(0x00);
  SPI_interface->transfer(0x00);
  switchCSpin(HIGH);

  // now send 5 dummy bytes to read the response back

  switchCSpin(LOW);
  status_response = transfer(addressByte);
  out = SPI_interface->transfer(0x00);
  out <<= 8;
  out |= SPI_interface->transfer(0x00);
  out <<= 8;
  out |= SPI_interface->transfer(0x00);
  out <<= 8;
  out |= SPI_interface->transfer(0x00);
  switchCSpin(HIGH);

  SPI_interface->endTransaction();  // release settings for other routines

  return out;
}

void DumbledoreTMC::deactivateCSPins(TCA9535& t_selector, uint8_t t_cs_index) {
  static std::set<uint8_t> cs_indices;
  if (!cs_indices.contains(t_cs_index)) {
    cs_indices.emplace(t_cs_index);
  }

  for (const auto& cs_index : cs_indices) {
    t_selector.writePin(cs_index, HIGH);
  }
}
