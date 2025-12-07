### RPI Simon-Inspired Pattern Game

A Raspberry Pi controlled Simon-inspired pattern game. Match the
directions in the correct sequence to continue to the next level.

Created by [Miles Murgaw](https://github.com/DogeDark) as a final project for CNIT 105 @ Purdue Polytechnic.

### Features:

- 8x8 LED Matrix for displaying directions, and other effects. Main display.
- LED Bar for displaying current level.
- Passive buzzer for audio effects and tunes.
- XY + Z Button Joystick for matching patterns. Primary input.

### Project Organization:

This project is organized into a handful of files.
- main.c contains the main game logic and control flow.
- led_matrix.c contains the led matrix rendering logic.
- led_bar.c contains the led bar rendering logic.
- buzzer.c contains the tunes and audio effect logic.

### Compiling
This project is meant to be compiled on a Raspberry Pi. This has only been tested on an RPI 4B.

Debug:
```bash
gcc src/main.c src/led_matrix.c src/led_bar.c src/buzzer.c src/joystick.c -o game -lwiringPi -lpthread
```
Release:
```bash
gcc src/main.c src/led_matrix.c src/led_bar.c src/buzzer.c src/joystick.c -o game -lwiringPi -lpthread -O3 -DNDEBUG -march=native -mtune=native
```