# SEALC
# System for Electro-Acoustic Live Coding

# Description
Microcontroller program to live code motors (steppers, servos and vibros) through a serial terminal.

 [motor# (0-9)]command[argument]

Notes:
- Separate several commands by comma to send them at once.
- No need to re-select motor once selected.

# Commands:
- SS set speed: argument in RPM (default 12RPM).

- SD set direction: argument=0 => CW /argument>0 => CCW (default 0).

- ST stop motion: no argument.

- SA stop all: stop all motors;

- RO rotate: for steppers, no argument or argument=0 => continuously / argument>0 => argument is number of turns. For vibros, no argument or argument=0 => continuously ON / argument>0 => argument is ON duration.

- RW rotate wave: rotate continuously with triangle speed variation (0-speed-0). Argument is number of periods per turn. Steppers only.

- RA rotate absolute: argument is angle in degrees. Same as RR for steppers.

- RR rotate relative: argument is angle in degrees. Only for servos.

- RP rotate pause: for steppers, argument is [turns:pause], no argument or argument=0 => 1 turn, 1s pause. Rotate a number of turns then pause, rotate, pause, etc. For vibros, argument is [duration ON:duration OFF], no argument or argument=0 => 1s ON, 1s OFF.

- SQ sequence: for servos and steppers, argument is [angle:(0-2):...:(0-2)], sweep-like motion of angle. 0 means no motion (duration of sweep), 1 is sweep motion in current direction, 2 is sweep motion in opposite current direction. Up to 10 steps. For vibros, argument is [duration:state:...:duration:state]. Duration in ms, state is ON (1) or OFF (0).

- GS: get speed in RPM.

- GD: get direction.

- GM: get mode.

- GI: get selected motor id and type.

- WA: wait: argument is wait time in ms.


