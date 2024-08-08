# HexTech Muscle User Manual

Welcome to the HexTech Circuit Board User Manual. This guide will help you get started with the HexTech circuit board, a versatile platform for Rapid Product Development. 

## Table of Contents
- [Introduction](#introduction)
- [Getting Started](#getting-started)
- [Setting Up the Hardware](#Setting-Up-the-Hardware)
- [Connecting power, motors, and other peripherals](#connecting-power,-motors,-and-other-peripherals)
- [Installation](#installation)
- [Using the HexTech Library](#using-the-hextech-library)
- [Programming with HexTech](#programming-with-hextech)
- [Example Projects](#example-projects)
- [Troubleshooting](#troubleshooting)
- [Conclusion](#conclusion)

## Introduction
Whether you're a beginner or an experienced engineer, HexTech provides the tools you need to bring your ideas to life. With built-in LEDs and the ability to control stepper motors, DC motors, BLDC motors, MOSFETs, switches, sensors, solenoids, and more, HexTech is a powerful and user-friendly tool for a wide range of projects. Whether you're looking to automate your home, build a robot, or control devices remotely, HexTech provides the flexibility and ease of use needed to bring your ideas to life.

## Getting Started
### What You Need
- HexTech circuit board starter kit:
  - HexTech circuit board (provided)
  - Power supply (provided)
  - Power cables (provided)
  - 2.5mm tip width flat head screwdriver (provided)
  - USB cable for connection to your computer (provided)
- Motors (stepper, DC, BLDC), switches, sensors, solenoids, and other peripherals as needed

## Setting Up the Hardware
- Connect the HexTech board to your computer using the USB cable.
      - It will show up on your computer as a serial device. (on windows it will show up as a COM) 
- Attach any motors, switches, sensors, solenoids, or other peripherals to the appropriate pins on the HexTech board.

## Connecting power, motors, and other peripherals 

1. Use the provided 2.4 mm tip screwdriver to loosen the screw.
<img width="989" alt="Screenshot 2024-08-07 at 5 04 00 PM" src="https://github.com/user-attachments/assets/6673893f-e947-4e89-b78c-68231dbdbaac">

  
2. Insert the cable into the terminal block.
<img width="773" alt="Screenshot 2024-08-07 at 5 04 21 PM" src="https://github.com/user-attachments/assets/90e4dd46-ba35-4ed1-9b68-81141cb68c72">

   
3. Tighten the screw securely with the screwdriver.
<img width="936" alt="Screenshot 2024-08-07 at 5 03 44 PM" src="https://github.com/user-attachments/assets/caea1f95-9685-4213-b72c-a8fe669fc9b4">

## Installation
To use the HexTech board with Python, you need to set up the HexTech library. Follow these steps:

1. Clone the repository:
   ```sh
   git clone https://github.com/your-username/hextech.git
   cd hextech
2. Install the required Python packages:
   ```sh
    pip install pyserial keyboard paho-mqtt


## Using the HexTech Library

To get started with programming the HexTech board, import the `HexTechMusclev1` class from the library:

```{python}
from hextech import HexTechMusclev1

hex = HexTechMusclev1("COM10")
hex.blue_led.turn_on()
```


## Parallel and Sequential Execution

To execute multiple commands at the same time or in sequence, use the `execute_in_parallel` and `execute_in_sequence` methods:

```{python}

from hextech import HexTechMusclev1

hex = HexTechMusclev1("COM5")

hex.execute_in_parallel([
    hex.blue_led.turn_on(False),
    hex.stepper0.move(2000, False)
])
```

## Programming with HexTech

### Controlling LEDs

The HexTech board has built-in LEDs that can be controlled using the `LED` class. Here are some basic commands:

* Turn on the blue LED:
  ```{python}
  hex.blue_led.turn_on()
  ```
* Turn off the blue LED:

   ```{python}
    hex.blue_led.turn_off()
   ```

* Flash the blue LED:
   ```{python}
    hex.blue_led.flash()
   ```
### Controlling Stepper Motors

The HexTech board can control up to four stepper motors. Here are some basic commands:

* Set speed:
    ```{python}
    hex.stepper0.set_speed(1000)
    ```

* Move the motor:
    ```{python}
    hex.stepper0.move(200)
    ```

## Controlling DC and BLDC Motors

The HexTech board can control one DC motor and one BLDC motor. Here are some basic commands:

* Move the DC motor:
  
  ```{python}
  hex.dc.move(10)
  ```

* Stop the DC motor:

  ```{python}
  hex.dc.stop()
  ```
  
## Controlling MOSFETs

The HexTech board can control three MOSFETs. Here are some basic commands:

* Set duty cycle:

   ```{python}
   hex.mos0.set_duty(512)
   ```
* Set frequency:

  ```{python}
  hex.mos0.set_frequency(1000)
  
  ```
* Turn on the MOSFET:

  ```{python}
  hex.mos0.turn_on()
  ```
* Turn off the MOSFET:

  ```{python}
  hex.mos0.turn_off()
  ```

  ## Example Projects

### Automatic Cat Feeder

Build an automatic cat feeder that dispenses food at scheduled times using the HexTech board to control a stepper motor.
[Automatic Cat Feeder Repository](https://github.com/andrea-lk/automatic-catfeeder)

### Automatic Wire Stripper

Create an automatic wire stripper that can precisely strip insulation from wires of various sizes using the HexTech board to control the necessary motors and sensors.
[HexTechWireStripper Repository](https://github.com/justinebanks/HexTechWireStripper)

### Automatic Pencil Sharpener

Develop an automatic pencil sharpener that sharpens pencils to a perfect point every time using the HexTech board to control the motors and sensors.
[HexTechAutomaticPencilSharpener Repository](https://github.com/FRatmiroff/HexTechAutomaticPencilSharpener)

### Robotic Arm
[HexTechRobotArm Repository](https://github.com/FRatmiroff/HexTechRobotArm)

### Crate Mover
Create a crate on wheels. 
[HexTechMover Repository](https://github.com/justinebanks/HexTechMover)

### CNC Machine



### Smart Home Automation

Use the HexTech board to control lights, fans, and other devices in your home, creating a smart home automation system.

## Conclusion

The HexTech circuit board is a powerful and versatile tool for anyone interested in electronics and automation. With its user-friendly interface and robust functionality, HexTech makes it easy to create, control, and automate a wide range of projects. Automate your house, build robots, or create smart devices—the possibilities are endless.


