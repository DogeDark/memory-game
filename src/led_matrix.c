/*

The 8x8 matrix module has two shift registers daisy chained.
To set both rows and columns, you must push two bytes.
The first byte is for the row, and the second is for the column.
The column's byte is inverted. So `1` is off, `0` is on.

The 8x8 matrix cannot turn on and off individual leds - it works based on rows and colums.
We must instead cycle through each "pixel", quickly turning on then back off the corresponding LED,
to create the illusion of an actual picture.

*/

#include <stdio.h>
#include <wiringPi.h>
#include <wiringShift.h>
#include <pthread.h>
#include <string.h>

// Frame definitions for LED Matrix

const int BLANK[8][8] = {};

// ←
const int ARROW_LEFT[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 0},
    {0, 0, 1, 1, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 0, 0, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
};

// →
const int ARROW_RIGHT[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 1, 1, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
};

// ↑
const int ARROW_UP[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 1, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
};

// ↓
const int ARROW_DOWN[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 1, 1, 1, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
};

// X
const int INCORRECT[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 0},
    {0, 0, 1, 0, 0, 1, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 1, 0, 0, 1, 0, 0},
    {0, 1, 0, 0, 0, 0, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
};

// ʘ
const int READY[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 0, 1, 0},
    {0, 1, 0, 1, 1, 0, 1, 0},
    {0, 1, 0, 1, 1, 0, 1, 0},
    {0, 1, 0, 0, 0, 0, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
};


// Pin definitions
#define LATCH 6
#define CLK 10
#define DATA 11

// Prototypes
static void *render(void *arg);
static void pushByte(unsigned char byte);
static void workMatrixFrame(int frame[8][8]);

// The current frame to be rendered.
//
// Safety: This global is used across two threads. One that sets it, and one that reads it.
// Race conditions are acceptable as the effects do not cause issue to any logic, it is set-and-forget.
static int currentFrame[8][8] = {};

// Initialize the LED matrix and start the rendering thread.
void ledMatrixInit()
{
    // Set pins
    pinMode(LATCH, OUTPUT);
    pinMode(CLK, OUTPUT);
    pinMode(DATA, OUTPUT);

    // Start LED Matrix led thread.
    pthread_t render_thread;
    pthread_create(&render_thread, NULL, render, NULL);
}

// Set the matrix to a new frame.
void ledMatrixSetFrame(const int frame[8][8])
{
    memcpy(currentFrame, frame, sizeof(currentFrame));
}

// Pushes a byte to the matrix shift registers.
static void pushByte(unsigned char byte)
{
    digitalWrite(LATCH, LOW);
    shiftOut(DATA, CLK, MSBFIRST, byte);
    digitalWrite(LATCH, HIGH);
}

// Starts the rendering loop to render complete frames.
static void *render(void *arg)
{
    printf("LED Matrix Render Thread Started\n");
    while (1)
    {
        workMatrixFrame(currentFrame);
    }
}

// Work an entire frame to the 8x8 matrix.
// The matrix cannot enable/disable individual leds
// We must work entire rows/columns VERY fast to creat the illusion of a full picture.
static void workMatrixFrame(int frame[8][8])
{

    for (int row_i = 0; row_i < 8; row_i++)
    {
        // Turn off all LEDs to make way for the next pixel.
        pushByte(0);
        pushByte(0);

        unsigned char row = 0b0;
        unsigned char col = 0b11111111;

        for (int col_i = 0; col_i < 8; col_i++)
        {
            int cell = frame[row_i][col_i];

            // If the cell is enabled, we enable the row and col for the matrix.
            if (cell)
            {
                switch (row_i)
                {
                case 0:
                    row = 0b10000000;
                    break;
                case 1:
                    row = 0b01000000;
                    break;
                case 2:
                    row = 0b00100000;
                    break;
                case 3:
                    row = 0b00010000;
                    break;
                case 4:
                    row = 0b00001000;
                    break;
                case 5:
                    row = 0b00000100;
                    break;
                case 6:
                    row = 0b00000010;
                    break;
                case 7:
                    row = 0b00000001;
                    break;
                }

                switch (col_i)
                {
                case 0:
                    col = 0b01111111;
                    break;
                case 1:
                    col = 0b10111111;
                    break;
                case 2:
                    col = 0b11011111;
                    break;
                case 3:
                    col = 0b11101111;
                    break;
                case 4:
                    col = 0b11110111;
                    break;
                case 5:
                    col = 0b11111011;
                    break;
                case 6:
                    col = 0b11111101;
                    break;
                case 7:
                    col = 0b11111110;
                    break;
                }

                pushByte(row);
                pushByte(col);

                // Wait so the LED has time to turn on.
                delayMicroseconds(100);
            }
        }
    }
}