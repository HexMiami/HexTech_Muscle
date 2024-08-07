# HexTech_Muscle
HexTech Muscle for Rapid Product Development


# HexTech Circuit Board User Manual

Welcome to the HexTech Circuit Board User Manual. This guide will help you get started with the HexTech circuit board, a versatile platform for building and automating various projects.

## Table of Contents
- [Introduction](#introduction)
- [Getting Started](#getting-started)
- [Connecting power, motors, and other peripherals](#connecting-power,-motors,-and-other-peripherals)
- [Installation](#installation)
- [Using the HexTech Library](#using-the-hextech-library)
- [Connecting to MQTT](#connecting-to-mqtt)
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
- Motors (stepper, DC, BLDC), switches, sensors, solenoids, and other peripherals as needed

## Connecting power, motors, and other peripherals 

1. Use the provided 2.5mm tip screwdriver to loosen the screw.

  
2. Insert the cable into the terminal block.

   
3. Tighten the screw securely with the screwdriver.




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

 

## Connecting to MQTT

Connect to HexTech from your computer using MQTT. Here is an example of how to connect to the HexTech board through Python using the `paho-mqtt` library:

```{python}
    import time
    import paho.mqtt.client as mqtt
    
    USERNAME = "hextech-andrea"
    PASSWORD = "andrea"
    commands = "hextech/hextech-andrea/commands"
    topic = "hextech/hextech-andrea/status"
    
    def on_publish(client, userdata, mid, reason_code, properties):
        try:
            userdata.remove(mid)
            print("Published")
        except KeyError:
            print("on_publish() is called with a mid not present in unacked_publish")
            print("This is due to an unavoidable race-condition:")
            print("* publish() returns the mid of the message sent.")
            print("* mid from publish() is added to unacked_publish by the main thread")
            print("* on_publish() is called by the loop_start thread")
            print("While unlikely (because on_publish() will be called after a network round-trip),")
            print(" this is a race-condition that COULD happen")
            print("")
            print("The best solution to avoid race-condition is using the msg_info from publish()")
            print("We could also try using a list of acknowledged mid rather than removing from pending list,")
            print("but remember that mid could be re-used!")
    
    def on_connect(client, userdata, flags, reason_code, properties):
        print(f"Connected with result code {reason_code}")
        client.subscribe(topic)
    
    def on_message(client, userdata, msg):
        print(f"Received message '{msg.payload.decode()}' on topic '{msg.topic}'")
    
    unacked_publish = set()
    
    mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    mqttc.on_publish = on_publish
    mqttc.on_connect = on_connect
    mqttc.on_message = on_message
    mqttc.user_data_set(unacked_publish)
    mqttc.username_pw_set(username=USERNAME, password=PASSWORD)
    mqttc.connect("mqtt.hextronics.cloud", 1883)
    mqttc.loop_start()
    
    msg_info = mqttc.publish(commands, "led.bl_off", 0, False)
    unacked_publish.add(msg_info.mid)
    
    while len(unacked_publish):
        time.sleep(0.1)
    
    msg_info.wait_for_publish()
    mqttc.disconnect()
    mqttc.loop_stop()
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

### Automatic Wire Stripper

Create an automatic wire stripper that can precisely strip insulation from wires of various sizes using the HexTech board to control the necessary motors and sensors.

### Automatic Pencil Sharpener

Develop an automatic pencil sharpener that sharpens pencils to a perfect point every time using the HexTech board to control the motors and sensors.

### Smart Home Automation

Use the HexTech board to control lights, fans, and other devices in your home, creating a smart home automation system.

## Conclusion

The HexTech circuit board is a powerful and versatile tool for anyone interested in electronics and automation. With its user-friendly interface and robust functionality, HexTech makes it easy to create, control, and automate a wide range of projects. Automate your house, build robots, or create smart devices—the possibilities are endless.


