from __future__ import annotations

from enum import IntEnum, StrEnum
from typing import cast

import serial
import serial.tools.list_ports
from serial.tools.list_ports_common import ListPortInfo


def _detect_board() -> str:
    devices = cast(list[ListPortInfo], serial.tools.list_ports.comports())
    boards = list(filter(
        lambda e: e.vid == 0x303A and e.pid == 0x1001, devices))

    if len(boards) == 0:
        raise Exception("No HexTech device detected")
    elif len(boards) > 1:
        raise UserWarning("Multiple HexTech devices detected, choosing first")

    return boards[0].device


class Command:
    def __init__(self, board: HexTech, msg: str) -> None:
        self.board = board
        self.msg = msg

    def run(self) -> None:
        self.board._send(self.msg)

    def __str__(self) -> str:
        return self.msg

    def __bytes__(self) -> bytes:
        return self.msg.encode()


class CommandGroup:
    def run(self, board: HexTech) -> None:
        board._send(self.compose())

    def compose(self) -> str:
        raise NotImplementedError()


class CommandGroupSequential(CommandGroup):
    def __init__(self, *cmds: Command) -> None:
        self.cmds = cmds

    def compose(self) -> str:
       return '|'.join(map(str, self.cmds))


class CommandGroupParallel(CommandGroup):
    def __init__(self, *cmds: Command) -> None:
        self.cmds = cmds

    def compose(self) -> str:
        return ';'.join(map(str, self.cmds))


class HexTech:
    def __init__(
        self,
        device: str | None = None,
        verbose: bool = False,
    ) -> None:
        self.device = _detect_board() if device is None else device
        self.conn = serial.Serial(
            port=self.device,
            baudrate=115200,
            timeout=0.1,
        )
        self.verbose = verbose

    def _send(self, msg: str) -> None:
        if self.verbose:
            print(f"  > {msg}")
        self.conn.write(f'{msg}\n'.encode())

    def run(self, cmd: Command | CommandGroup) -> None:
        if isinstance(cmd, CommandGroup):
            cmd.run(self)
        elif isinstance(cmd, Command):
            cmd.run()


    class LED:
        def __init__(self, board: HexTech, color: LEDPin):
            self.board = board
            self.color = color
            self.pin = color.value # LED Pin: bl, wh, yl
            self.state: LEDState = LEDState.OFF

        def turn_on(self) -> Command:
            self.state = LEDState.ON
            return Command(self.board, f"led.{self.pin}_on")

        def turn_off(self) -> Command:
            self.state = LEDState.OFF
            return Command(self.board, f"led.{self.pin}_off")

        def flash(self) -> Command:
            self.state = LEDState.FLASHING
            return Command(self.board, f"led.{self.pin}_flash")


    class StepperMotor:
        def __init__(self, board: HexTech, pin_number: int):
            self.board = board
            self.pin_number = pin_number
            self.pin =  "0" + str(pin_number)

        def set_speed(self, value: int) -> Command:
            return Command(self.board, f"stepper.{self.pin}_speed_{value}")

        def set_microsteps(self, microsteps: Microsteps) -> Command:
            return Command(self.board,
                f"stepper.{self.pin}_microsteps_{microsteps}")

        def set_acceleration(self, accel: int) -> Command:
            return Command(self.board, f"stepper.{self.pin}_accel_{accel}")

        def set_current(self, current: int) -> Command:
            return Command(self.board, f"stepper.{self.pin}_rms_{current}")

        # Moves the Stepper the Specified Number of Steps
        def move(self, steps: int, stealth: bool = True) -> Command:
            return Command(self.board,
                f"stepper.{self.pin}_move_{steps}"
                + ("_0" if not stealth else ""))

        # Goes to Specified Position
        def go(self, position: int, stealth: bool = True) -> Command:
            return Command(self.board,
                f"stepper.{self.pin}_go_{position}"
                + ("_0" if not stealth else ""))

        def stop(self, force: bool = True) -> Command:
            return Command(self.board,
                f"{"P" if force == True else ""}_stepper.{self.pin}_stop")


    class DCMotor:
        def __init__(self, board: HexTech, pin_number: int) -> None:
            self.board = board
            self.pin_number = pin_number
            self.pin = "0" + str(pin_number)

        def move(self, seconds: int) -> Command:
            return Command(self.board, f"dc.{self.pin}_move_{seconds}")

        def stop(self, force: bool = True) -> Command:
            return Command(self.board,
                f"{"P" if force == True else ""}_dc.{self.pin}_stop")


    class BLDCMotor:
        def __init__(self, board: HexTech, pin_number: int) -> None:
            self.board = board
            self.pin_number = pin_number
            self.pin = "0" + str(pin_number)

        def move(self, seconds: int) -> Command:
            return Command(self.board, f"bldc.{self.pin}_move_{seconds}")

        def stop(self, force: bool = True) -> Command:
            return Command(self.board,
                f"{"P" if force == True else ""}_bldc.{self.pin}_stop")


    class MOSFET:
        def __init__(self, board: HexTech, pin_number: int) -> None:
            self.board = board
            self.pin_number = pin_number
            self.pin = "0" + str(pin_number)

        def set_duty(self, duty_cycle: int) -> Command:
            """Set duty cycle, 0 <= value <= 1023"""
            return Command(self.board, f"mos.{self.pin}_duty_{duty_cycle}")

        def set_frequency(self, value: int) -> Command:
            return Command(self.board, f"mos.{self.pin}_freq_{value}")

        def turn_on(self) -> Command:
            return Command(self.board, f"mos.{self.pin}_on")

        def turn_off(self) -> Command:
            return Command(self.board, f"mos.{self.pin}_off")


class HexTechMuscle(HexTech):
    def __init__(self, device: str | None = None, verbose: bool = False):
        super().__init__(device, verbose)

        self.stepper0 = self.StepperMotor(self, 0)
        self.stepper1 = self.StepperMotor(self, 1)
        self.stepper2 = self.StepperMotor(self, 2)
        self.stepper3 = self.StepperMotor(self, 3)

        self.blue_led = self.LED(self, LEDPin.BLUE)
        self.white_led = self.LED(self, LEDPin.WHITE)
        self.yellow_led = self.LED(self, LEDPin.YELLOW)

        self.dc = self.DCMotor(self, 0)
        self.bldc = self.BLDCMotor(self, 0)

        self.mos0 = self.MOSFET(self, 0)
        self.mos1 = self.MOSFET(self, 1)
        self.mos2 = self.MOSFET(self, 2)


class LEDPin(StrEnum):
    BLUE = "bl"
    WHITE = "wh"
    YELLOW = "yl"


class LEDState(StrEnum):
    OFF = "off"
    ON = "on"
    FLASHING = "flashing"


class Microsteps(IntEnum):
    MS_2 = 2
    MS_4 = 4
    MS_8 = 8
    MS_16 = 16
    MS_32 = 32
    MS_64 = 64
    MS_128 = 128
    MS_256 = 256
