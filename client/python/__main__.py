from enum import Enum
from typing import override

import serial
import keyboard
import threading
import time


# The Thread Started By The 'HexTech.__init__' Function to Keep the Connection Open Until the Program Is Terminated
def connection_thread(ser, verbose=False):
    print("Serial Communication Begin")
    running = True

    while running:
        if ser.in_waiting:
            packet = ser.readline()

            if verbose:
                print(packet.decode("utf-8"))
            
        if keyboard.is_pressed("Ctrl+c"):
            print("Serial Communication End")
            running = False

    ser.close()


# Simple Utility Function to Test HexTech Commands With
def command_loop(conn: serial.Serial):
    while (True):
        command = input("Input Command: ")
        HexTech.execute_command(conn, command)



class LEDPin(Enum):
    BLUE = "bl"
    WHITE = "wh"
    YELLOW = "yl"


class HexTech:
    def __init__(self, serial_port: str, verbose=False):
        # Creates the a Serial Object and Starts a New Thread to Keep the Connection Opened
        self.conn = serial.Serial(serial_port, 115200)
        com_thread = threading.Thread(target=connection_thread, args=[self.conn, verbose])
        com_thread.start()


    # Wraps String Command in the 'bytes' Function For Simplicity
    @staticmethod
    def execute_command(conn, command):
        conn.write(bytes(command, 'utf-8'))
        print(f"EXECUTED COMMAND: {command}")
    

    # Note: HexTech.sleep might not factor in the amount of time it takes to actually execute the previous command
    @staticmethod
    def sleep(duration: int):
        return "sleep_" + str(duration)
    

    # Note: The Functions 'execute_in_parallel' & 'execute_in_sequence' cannot be nested within each other
    def execute_in_parallel(self, commands: list):
        command_string = ""

        for command in commands:
            command_string += command + ";"
        
        self.execute_command(self.conn, command_string)
        return command_string


    def execute_in_sequence(self, commands: list):
        command_string = ""

        for command in commands:
            if "sleep" not in command:
                command_string += command + "|"
            else:
                sleep_duration = int(command.split("_")[1])
                self.execute_command(self.conn, command_string)
                command_string = ""
                time.sleep(sleep_duration)
        
        self.execute_command(self.conn, command_string)
        return command_string


    class LED:
        def __init__(self, conn: serial.Serial, color: LEDPin):
            self.color = color.name
            self.pin = color.value # LED Pin: bl, wh, yl
            self.conn = conn
            self.current_state = "off"


        def turn_on(self, exec=True):
            self.current_state = "on"
            command = f"led.{self.pin}_on"

            if exec: HexTech.execute_command(self.conn, command)
            return command


        def turn_off(self, exec=True):
            self.current_state = "off"
            command = f"led.{self.pin}_off"

            if exec: HexTech.execute_command(self.conn, command)
            return command


        def flash(self, exec=True):
            self.current_state = "flashing"
            command = f"led.{self.pin}_flash"

            if exec: HexTech.execute_command(self.conn, command)
            return command


    class StepperMotor:
        def __init__(self, conn: serial.Serial, pin_number: int):
            stepper_pins = ["00", "01", "02", "03"]

            self.pin_number = pin_number
            self.pin =  "0" + str(pin_number) # stepper_pins[self.pin_number]
            self.conn = conn
            self.stealth = 1 # 1 => Checks Stall Guard, 0 => Does Not
        
        def set_speed(self, value: int, exec=True):
            command = f"stepper.{self.pin}_speed_{value}"

            if exec: HexTech.execute_command(self.conn, command)
            return command


        # Sets The Microstep Divisor (Ex. 2 => Half-Step, 4 => Quarter-Step, ..., 256)
        def set_microsteps(self, value: int, exec=True):
            possible_values = [2, 4, 8, 16, 32, 64, 128, 256]
            command = f"stepper.{self.pin}_microsteps_{value}"

            if value in possible_values:
                if exec: HexTech.execute_command(self.conn, command)
                return command
            else:
                raise ValueError("Value Passed to 'set_microsteps' Must Be 2, 4, 8, 16, 32, 64, 128, or 256")
        

        def set_acceleration(self, value: int, exec=True):
            command = f"stepper.{self.pin}_accel_{value}"
           
            if exec: HexTech.execute_command(self.conn, command)
            return command
        

        def set_current(self, current: int):
            command = f"stepper.{self.pin}_rms_{current}"

            if exec: HexTech.execute_command(self.conn, command)
            return command
        

        # Moves the Stepper the Specified Number of Steps
        def move(self, steps: int, exec=True):
            command = f"stepper.{self.pin}_move_{steps}"

            if exec: HexTech.execute_command(self.conn, command)
            return command


        # Goes to Specified Position
        def go(self, position: int, exec=True):
            command = f"stepper.{self.pin}_go_{position}"

            if exec: HexTech.execute_command(self.conn, command)
            return command
        

        def stop(self, force=True, exec=True):
            command = f"{"P" if force == True else ""}_stepper.{self.pin}_stop"

            if exec: HexTech.execute_command(self.conn, command)
            return command
        

    class DCMotor:
        def __init__(self, conn, pin_number):
            self.conn = conn
            self.pin_number = pin_number
            self.pin = "0" + str(pin_number)
        

        def move(self, seconds: int, exec=True):
            command = f"dc.{self.pin}_move_{seconds}"

            if exec: HexTech.execute_command(self.conn, command)
            return command


        def stop(self, force=True, exec=True):
            command = f"{"P" if force == True else ""}_dc.{self.pin}_stop"

            if exec: HexTech.execute_command(self.conn, command)
            return command


    class BLDCMotor(DCMotor):
        def __init__(self, conn, pin_number):
            super().__init__(conn, pin_number)
        

        @override
        def move(self, seconds: int, exec=True):
            command = f"bldc.{self.pin}_move_{seconds}"

            if exec: HexTech.execute_command(self.conn, command)
            return command

        @override
        def stop(self, force=True, exec=True):
            command = f"{"P" if force == True else ""}_bldc.{self.pin}_stop"

            if exec: HexTech.execute_command(self.conn, command)
            return command


    class MOSFET:
        def __init__(self, conn, pin_number):
            self.conn = conn
            self.pin_number = pin_number
            self.pin = "0" + str(pin_number)

        # The Duty for PWM (0 - 1023)
        def set_duty(self, value: int, exec=True):
            command = f"mos.{self.pin}_duty_{value}";

            if exec: HexTech.execute_command(self.conn, command)
            return command

        # PWM Frequency
        def set_frequency(self, value: int, exec=True):
            command = f"mos.{self.pin}_freq_{value}";

            if exec: HexTech.execute_command(self.conn, command)
            return command


        def turn_on(self, exec=True):
            command = f"mos.{self.pin}_on";

            if exec: HexTech.execute_command(self.conn, command)
            return command


        def turn_off(self, exec=True):
            command = f"mos.{self.pin}_off";

            if exec: HexTech.execute_command(self.conn, command)
            return command


class HexTechMusclev1(HexTech):
    def __init__(self, serial_port: str, verbose=False):
        super().__init__(serial_port, verbose)

        @staticmethod
        def execute_command():
            print("EXECUTED")

        self.stepper0 = self.StepperMotor(self.conn, 0)
        self.stepper1 = self.StepperMotor(self.conn, 1)
        self.stepper2 = self.StepperMotor(self.conn, 2)
        self.stepper3 = self.StepperMotor(self.conn, 3)

        self.blue_led = self.LED(self.conn, LEDPin.BLUE)
        self.white_led = self.LED(self.conn, LEDPin.WHITE)
        self.yellow_led = self.LED(self.conn, LEDPin.YELLOW)

        self.dc = self.DCMotor(self.conn, 0)
        self.bldc = self.BLDCMotor(self.conn, 0)

        self.mos0 = self.MOSFET(self.conn, 0)
        self.mos1 = self.MOSFET(self.conn, 1)
        self.mos2 = self.MOSFET(self.conn, 2)


class HexTechMusclev2(HexTech):
    pass


if __name__ == "__main__":
    HexTechMusclev1("/dev/ttyACM1")
