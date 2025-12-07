/*
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

```bash
gcc main.c led_matrix.c led_bar.c buzzer.c -o final -lwiringPi -lpthread
```
*/

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <time.h>
#include <stdlib.h>

#include "led_matrix.h"
#include "led_bar.h"
#include "buzzer.h"
#include "joystick.h"

void startMenu();
void game();
void displayPattern(int pattern);
int rand_range(int min, int max);
void interruptHandler(const int _signal);

int main(void)
{
	// Initialize wiring pi
	printf("Init Wiring Pi\n");
	signal(SIGINT, interruptHandler);
	if (-1 == wiringPiSetup())
	{
		printf("Failed to setup Wiring Pi!\n");
		return 1;
	}

	// Init periphs
	printf("Init Periphs\n");

	srand((unsigned)time(NULL));
	ledMatrixInit();
	ledBarInit();
	buzInit();
	joystickInit();

	printf("Success\n");

	// Startup tone
	buzPlaySuccess();
	delay(1000);

	printf("Initialized\n");

	// Game loop
	while (1)
	{
		// Set matrix to ready frame
		ledMatrixSetFrame(READY);
		delay(500);

		// Check if joystick is held down, and if so, start game.
		if (joystickZedDown())
		{
			printf("Joystick pressed, starting game.\n");
			game();
		}
	}

	return 0;
}

void startMenu() {}

#define LEFT_PATTERN 0
#define RIGHT_PATTERN 1
#define UP_PATTERN 2
#define DOWN_PATTERN 3
#define INVALID_PATTERN -1

void game()
{
	printf("Starting Game\n");
	buzPlayCountdown();

	int currentLevel = 0;
	int patternIndex = 0;
	int expectedPattern[20] = {INVALID_PATTERN};

	while (1)
	{
		// Get a new pattern
		int newPattern = rand_range(0, 3);
		expectedPattern[patternIndex] = newPattern;
		patternIndex++;

		// Show new list of patterns
		// TODO: Sound effects, increasing speed.
		for (int i = 0; i < patternIndex; i++)
		{
			displayPattern(expectedPattern[i]);

			// Play tone
			// TODO: This *might* cause issues because of the delays, we may need to run buzzer in a separate thread.
			// TODO: Individual tones for each direction.
			buzPlay(880, 50);

			// Delay between showing each pattern.
			// TODO: Increase this based on level.
			delay(500);
		}

		ledMatrixSetFrame(BLANK);

		// Test user's pattern memory skill
		int failed = 0;
		int numInputs = 0;

		while (numInputs < patternIndex)
		{
			// Wait for joystick input
			int input = joystickWaitForDir();
			printf("Input: %d\n", input);
			printf("Expected: %d\n", expectedPattern[numInputs]);

			// Test if input is correct
			if (expectedPattern[numInputs] != input)
			{
				failed = 1;
				break;
			}

			numInputs++;
			joystickWaitForCenter();
		}

		if (failed)
		{
			// TODO: Failed led effect.
			ledBarClear();
			ledMatrixSetFrame(INCORRECT);
			buzPlayIncorrect();
			break;
		}

		// Success! Increase level and update outputs.
		currentLevel++;
		buzPlaySuccess();

		for (int i = 0; i < currentLevel; i++)
			ledBarSet(i, LED_ON);

		// TODO: Max level (10) handling.
	}

	delay(3000);
	ledBarClear();
}

// Displays an arrow LED matrix frame based on supplied pattern.
void displayPattern(int pattern)
{
	printf("Showing pattern %d\n", pattern);
	switch (pattern)
	{
	case 0:
		ledMatrixSetFrame(ARROW_LEFT);
		break;
	case 1:
		ledMatrixSetFrame(ARROW_RIGHT);
		break;
	case 2:
		ledMatrixSetFrame(ARROW_UP);
		break;
	case 3:
		ledMatrixSetFrame(ARROW_DOWN);
		break;
	}
}

int rand_range(int min, int max)
{
	int num_numbers = max - min + 1;
	return (rand() % num_numbers) + min;
}

void interruptHandler(const int _signal)
{
	exit(0);
}