from hextech_muscle import CommandGroupSequential, HexTechMuscle

muscle = HexTechMuscle()

muscle.run(CommandGroupSequential(
    muscle.stepper0.set_speed(1000),
    muscle.stepper0.set_current(800),
    muscle.stepper0.set_acceleration(2000),
    muscle.stepper0.move(7000),
))

muscle.stepper0.move(-7000).run()
