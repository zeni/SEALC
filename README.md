# SEALC
# System for Electro-Acoustic Live Coding

# Description
Microcontroller program to live code motors (steppers and servos) through a serial terminal.

 [motor# (0-9)]command[argument]

Notes:
- Separate several commands by comma to send them at once.
- No need to re-select motor once selected.

# Commands:
- SS set speed: argument in RPM.

- SD set direction: argument=0 => CW /argument>0 => CCW.

- ST stop motion: no argument.

- RO rotate: no argument or argument=0 => continuously / argument>0 => argument is number of turns.

- RW rotate wave: rotate continuously with triangle speed variation (0-speed). Argument is number of periods per turn.

- RA rotate angle: argument is angle in degrees.

- SQ sequence: argument is [angle:(0-2):...:(0-2)], sweep-like motion of angle. 0 means no motion (duration of sweep), 1 is sweep motion in current direction, 2 is sweep motion in opposite current direction. Up to 10 steps.


