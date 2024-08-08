# HexTech Muscle V1.0 Package Docs
The HexTech Muscle v1.0 has 4 built-in LEDs and pins for 4 stepper motors, 1 DC motor, 1 BLDC motor, and 3 MOSFETs. The board can be connected to either through serial commands or an MQTT connection. This Python Package can be used to access the board through code over serial communication.

## Quick Start
To get a program setup, include this repository as a subdirectory of your project directory, name it hextech, and then simply import the HexTechMusclev1 class in a python file.
```python
from hextech import HexTechMusclev1
```
The constructor takes in the serial port that the board is connected to and an optional boolean value for whether or not to print messages recieved from the board in the console.

The HexTechMusclev1 class contains members for each pin or led on the board and each one of those members have methods that can be called to make the pin perform various tasks.
```python
from hextech import HexTechMusclev1

hex = HexTechMusclev1("COM10")
hex.blue_led.turn_on()
```
It's that simple to get your HexTech working, but there are a few other things to consider when sending serial commands in code. Two separate messages sent simultaneously result in undefined behavior, because the board can only process one message at a time. To execute multiple commands at the same time, use the `execute_in_parallel` method. Similarly, if you need to execute commands in sequence, use the `execute_in_sequence` method.
```python
from hextech import HexTechMusclev1

hex = HexTechMusclev1("COM5")

hex.execute_in_parallel([
    hex.blue_led.turn_on(False),
    hex.stepper0.move(2000, False)
])
```
Always remember to pass `False` as the last parameter of each command to tell the program that the actual execution of that command will be handled by a seperate function. If this isn't done, the command wil execute right away or it won't execute at all.

## API Documentation
- [HexTechMusclev1]("#hextech")
- [LED](#led)
- [Stepper Motor](#steppermotor)
- [DC / BLDC Motor](#dc)
- [MOSFET](#mosfet)

### HexTechMusclev1 {#hextech}
This is the main wrapper of all functions relating to pins on the HexTech v1 Board.

| Member | Class |
| ----------- | ----------- |
| stepper0 | StepperMotor |
| stepper1 | StepperMotor |
| stepper2 | StepperMotor |
| stepper3 | StepperMotor |
| blue_led | LED |
| yellow_led | LED |
| white_led | LED |
| dc | DCMotor |
| bldc | BLDCMotor |
| mos0 | MOSFET |
| mos1 | MOSFET |
| mos2 | MOSFET |

### LED 
The LED class represents the built-in LED lights on the board.
| Method | Description |
| ------ | ----------- |
| turn_on() |  |
| turn_off() |  |
| flash() |  |

### Stepper Motor
The StepperMotor class represents the pins labeled "Stepper 0" - "Stepper 3" on the board.
| Method | Description |
| ------ | ----------- |
| set_speed(val) | |
| set_microsteps(val) | |
| set_acceleration(val) | |
| set_current(val) | |
| move(steps) |  |
| go(pos) | |
| stop() | |

### DCMotor & BLDCMotor {#dc}
The DCMotor class is used for the pin labeled "DC MOT DRV 0" while the BLDCMotor class is used for the one labeled "BLDC"

| Method | Description |
| ------ | ----------- |
| move(val) | |
| stop() | |

### MOSFET
The MOSFET class is for the pins labeled "PWM MOS".

| Method | Description |
| ------ | ----------- |
| set_duty(val) | |
| set_frequency(val) | |
| turn_on() | |
| turn_off() | |


