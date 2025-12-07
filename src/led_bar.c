/*

The LED bar uses a MY9221SS chip, which has a 2-wire custom protocol for sending bytes.
We will use the 16 bit protocol (two bytes per led) to make it easy to assembly bytes.
We only have 10 leds, so the last 4 bytes (in 16 bit mode) will just be 0x00 (off).

*/

#include <stdio.h>
#include <wiringPi.h>
#include <wiringShift.h>

// TODO: Define these pins.
#define CLK 4
#define DATA 5

const unsigned char LED_ON = 0xFF;
const unsigned char LED_OFF = 0x00;
const unsigned char LED_HALF = LED_ON / 2;

void ledBarClear();
void ledBarSet(int led, unsigned char value);
void ledBarRefresh();
static void startFrame();
static void finalizeFrame();
static void pushByte(unsigned char byte);
static void latch();

// The current LED bar status.
static unsigned char currentLeds[10] = {0};
static int clkFlag = 0;

// Initialize the LED bar, setting it to a clear status.
void ledBarInit()
{
    pinMode(CLK, OUTPUT);
    pinMode(DATA, OUTPUT);

    digitalWrite(CLK, LOW);
    digitalWrite(DATA, LOW);

    // Send two-byte command mode.
    pushByte(0);
    pushByte(0);
    // shiftOut(DATA, CLK, MSBFIRST, 0);
    // shiftOut(DATA, CLK, MSBFIRST, 0);

    ledBarClear();
}

// Clear the LED bar.
void ledBarClear()
{
    startFrame();

    for (int i = 0; i < 10; i++)
    {
        currentLeds[i] = LED_OFF;
        pushByte(LED_OFF);
    }

    finalizeFrame();
}

// Set an led to be a specific value.
void ledBarSet(int led, unsigned char value)
{
    if (led < 0 || led > 9)
        return;

    currentLeds[led] = value;
    ledBarRefresh();
}

// Refreshes the led bar using the current led values.
void ledBarRefresh()
{
    startFrame();
    for (int i = 0; i < 10; i++)
        pushByte(currentLeds[i]);

    finalizeFrame();
}

static void startFrame()
{
    pushByte(0);
    pushByte(0);
}

// Finalizes the frame by sending the last four unused bytes, and latching.
static void finalizeFrame()
{
    pushByte(LED_OFF);
    pushByte(LED_OFF);
    latch();
}

// Pushes a byte to the led bar.
// We use 16 bit mode, so we "scale" the single byte into two in a non-linear way.
static void pushByte(unsigned char byte)
{
    // Merge the two bytes together to create 32 bits, with the last 16 unused, first 16 for two bytes.
    unsigned char second_byte = byte;
    unsigned int bits = (((unsigned int)byte << 8) | second_byte) << 16;

    // Send the first 16 MSBs for two bytes.
    for (int i = 0; i < 16; i++)
    {
        digitalWrite(DATA, (bits & 0x80000000) ? HIGH : LOW);
        digitalWrite(CLK, !clkFlag);
        clkFlag = !clkFlag;

        bits <<= 1;
        delayMicroseconds(20);
    }
}

// Applies the new data to the chip.
static void latch()
{
    digitalWrite(DATA, LOW);
    delayMicroseconds(500);

    for (int i = 0; i < 8; i++)
    {
        digitalWrite(DATA, LOW);
        delayMicroseconds(1);
        digitalWrite(DATA, HIGH);
        delayMicroseconds(1);
    }

    delayMicroseconds(500);
}