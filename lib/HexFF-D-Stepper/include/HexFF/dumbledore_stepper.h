#ifndef __DUMBLEDORE_STEPPER_H__
#define __DUMBLEDORE_STEPPER_H__

#include <Arduino.h>
#include <SPI.h>

#include <AccelStepper.h>
#include <MultiStepper.h>
#include <TMCStepper.h>

#include "HexFF/TCA9535.h"

/*
 * class InheritedClass : public BaseClass
 * :: calls the global variable instead of local. Define a function outside the class and used to
 * access static variables of class
 */
/**********************************************************************
 * This class provides a variant of the TMC2160Stepper class
 * to support the CS mechanism in the Dumbledore board
 */
class DumbledoreTMC : public TMC2160Stepper {
public:
  void begin();
  // DumbledoreTMC(uint8_t csIndex, SPIClass *spi_interface, TCA9535 &stepperSelector);
  DumbledoreTMC(uint8_t csIndex, SPIClass *spi_interface, TCA9535 &stepperSelector);
  void driveParams();
  // void stepInfo(int mSteps, int rmsCurr);
  uint32_t read(uint8_t addressByte);
  void stallGuardConf(int8_t B);
  uint16_t pollStallGuard(void);

private:
  // void stepInfo(int mSteps, int rmsCurr);
  uint8_t _csIndex;
  TCA9535 &selector;
  SPIClass *SPI_interface;
  void beginTransaction();
  void endTransaction();
  void switchCSpin(bool state);
  uint8_t transfer(const uint8_t data);
  void write(uint8_t addressByte, uint32_t config);

  static void deactivateCSPins(TCA9535& t_selector, uint8_t t_cs_index);
};

/*********************************************************************
 * This class provides a stepper level of abstraction to be easily used
 * in the Hextronics Dumbledore board
 */
class DumbledoreStepper : public AccelStepper {
public:
  DumbledoreStepper(TCA9535* dirSelectExpander, uint8_t dirSelectPin, uint8_t stepPin,
                    uint8_t CSIndex, SPIClass *spi_interface, TCA9535* stepperSlector);
  void begin();
  DumbledoreTMC driver;  // provides direct access to the TMC 2160 controlling this stepper
  /*^^^^^^^^^^^ this is critical*********/
  bool command(const String *const args, int n);
  inline uint8_t getStepPin() const { return _stepPin; };
  inline uint8_t getCSIndex() const { return _CSIndex; };
  inline const char *getStepperName() const { return stepperName.c_str(); };

  bool setZero(void);
  long getPosition(void);
  bool isBusy(void);
  long rememberPos(void);
  void zeroInit(void);
  void setZeroSpeed(float inputSpeed, short multiplier);
  float zeroSpeed;
  void stealth(bool action);
  bool sgON;
  bool watchStall();
  void stepperSetup(uint16_t rms, uint16_t microStepVal, uint16_t stallGuardThreshold,
                    float speedVal, float accVal, float zeroSpeedVal, TCA9535* buttonComms);
  bool getSGstatus(void);
  void resetStallZero(bool var);
  int _zeroDirection;
  uint16_t _microStepVal;
  int8_t _stallGuardThreshold;
  int _speedVal;
  int _accelVal;
  int _rms;
  void setZeroPin(int pinNumber);
  bool limit;
  bool checkDone(void);
  void idle(void);
  int getTOFF(void);
  uint8_t _toff;
  unsigned long zeroTime;
  unsigned long startTime;
  unsigned long zeroStartTime;
  bool hitPin;
  long startPos;
  unsigned long actionTime;
  unsigned long timeout;
  bool zeroFail;
  void setDir(short dirMultiplier);
  long zeroError;
  short driveDir;
  void handleDone(void);
  bool timeoutProtection;
  void setTimeoutProtection(void);
  void startMotor(void);
  void stopMotor(void);
  long distWent;
  String stepperNum;
  bool useAverageStall;
  void stallDetectAverage(bool);
  void stepName(const String &_stepperName, const String &_stepperNum);
  bool isHome();

  int stallCycleDelay;
  int zeroPin;
  TCA9535 *_dirSelectExpander;

  int second;
  long lastPos;
  uint8_t _dirSelectPin;
  uint8_t _stepPin;
  uint8_t _CSIndex;
  static void fwd();
  static void back();
  void step0(long phase) override;  // this is the actual function generating a step fwd or back
  float accelTime;
  uint16_t sgValue;
  int stallClock;
  uint16_t sgAverage;
  bool stallZero;
  short incrementerVal;
  bool active;
  bool endEvent;
  int baseStallThreshold;
  short sgUnlock;
  short idleLock;
  short _lastStallThreshold;
  bool _stalled;
  bool stillTrying;
  bool inactive;
  String stepperName;
  int stallDelay;
  unsigned long lastStallTime;
  unsigned long stallClockDelay;
  unsigned long elapsed;
  short stallIncrementer;
  uint16_t sgValueMax;
  uint16_t sgValueMin;
  bool enableStallStop = true;
  void endZero(void);
  unsigned long plugClockDelay;
  unsigned long lastPlugTime;
  bool OL_1;
  bool OL_2;
  bool OL_3;
  bool unplugged;
  bool firstZero;
  static const size_t numSgValueReadings = 10;
  std::array<uint16_t, numSgValueReadings> sgValueReadings{0};
  int sgValueReadingsIndex = -1;
  uint16_t sgValueReadingsTotal = 0;
};

#endif
