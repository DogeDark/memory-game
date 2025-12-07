/*

The Raspberry Pi does not have any ADCs, so we are unable to use the joystick directly.
Instead, we must use the provided ADC module (ADC0832) and poll the ADC for input changes.

The ADC0832 starts a transaction when CS pin is pulled low, a start bit, mode bit, and channel bit are sent.
Bits are only read from the DATA pin on rising edges (low -> high)

*/

#include <wiringPi.h>
#include <stdlib.h>

#define DATA 27
#define CLK 28
#define CS 29

// The pin for the joystick zed button.
#define JOYSTICK_Z 22

#define X_CHANNEL 0
#define Y_CHANNEL 1

const int JOY_LEFT = 0;
const int JOY_RIGHT = 1;
const int JOY_UP = 2;
const int JOY_DOWN = 3;

static unsigned char readByte();
static void sendBit(int bit);
static unsigned char readChannel(int channel);

void joystickInit()
{
    pinMode(CS, OUTPUT);
    pinMode(DATA, OUTPUT);
    pinMode(CLK, OUTPUT);

    digitalWrite(CS, HIGH);
    digitalWrite(DATA, LOW);
    digitalWrite(CLK, LOW);
}

// Check if the joystick Zed button axis is pushed down.
int joystickZedDown()
{
    return !digitalRead(JOYSTICK_Z);
}

// Pauses the thread and waits for a joystick direction, returning it.
int joystickWaitForDir()
{
    unsigned char lastXSample = 128;
    unsigned char lastYSample = 128;

    while (1)
    {
        unsigned char newXSample = readChannel(X_CHANNEL);
        delay(25);
        unsigned char newYSample = readChannel(Y_CHANNEL);
        delay(25);

        int xDiff = abs(lastXSample - newXSample);
        int yDiff = abs(lastYSample - newYSample);

        // Channel 0 (x) likes to jump to 255, so we ignore it.
        if (xDiff <= 10 && newXSample != 255)
        {
            if (newXSample > 240)
                return JOY_RIGHT;
            else if (newXSample < 15)
                return JOY_LEFT;
        }

        if (yDiff <= 10)
        {
            if (newYSample > 240)
                return JOY_UP;
            else if (newYSample < 15)
                return JOY_DOWN;
        }

        lastXSample = newXSample;
        lastYSample = newYSample;
    }
}

// Pauses the thread until the joystick is centered.
void joystickWaitForCenter()
{
    unsigned char lastXSample = 0;
    unsigned char lastYSample = 0;

    while (1)
    {
        unsigned char newXSample = readChannel(X_CHANNEL);
        delay(25);
        unsigned char newYSample = readChannel(Y_CHANNEL);
        delay(25);

        int xDiff = abs(lastXSample - newXSample);
        int yDiff = abs(lastYSample - newYSample);

        int xCentered = xDiff <= 10 && newXSample > 120 && newXSample < 130;
        int yCentered = yDiff <= 10 && newYSample > 120 && newXSample < 130;

        if (xCentered && yCentered)
            return;

        lastXSample = newXSample;
        lastYSample = newYSample;
    }
}

// Reads an ADC channel, returning the byte-value of data received.
static unsigned char readChannel(int channel)
{
    if (channel != 0 && channel != 1)
        return 0;

    // Pull CS low, send HIGH start bit, send mode bit, and send channel bit.
    digitalWrite(CLK, LOW);
    digitalWrite(CS, LOW);
    delayMicroseconds(20);
    sendBit(1);       // Start bit
    sendBit(1);       // Mode Bit (single ended = 1)
    sendBit(channel); // Channel bit (0 = ch0, 1 = ch1)

    // Send an extra clock pulse
    digitalWrite(CLK, HIGH);
    delayMicroseconds(20);
    digitalWrite(CLK, LOW);
    delayMicroseconds(20);

    // Read a byte representing that channel's ADC value.
    unsigned char data = readByte();

    // End transaction
    digitalWrite(CS, HIGH);
    delay(5);

    return data;
}

// Sends a single bit to the ADC.
static void sendBit(int bit)
{
    digitalWrite(DATA, bit);
    delayMicroseconds(20);
    digitalWrite(CLK, HIGH);
    delayMicroseconds(20);
    digitalWrite(CLK, LOW);
    delayMicroseconds(20);
}

// Reads a single byte from the ADC.
static unsigned char readByte()
{
    pinMode(DATA, INPUT);
    unsigned char data = 0;

    for (int i = 0; i < 16; i++)
    {
        // Pulse the clock so the ADC sets the next bit.
        digitalWrite(CLK, HIGH);
        delayMicroseconds(20);

        // The ADC sends MSBF byte first, then LSBF copy,
        // so we ignore the latter but need to get it out of the buffer.
        if (i > 7)
            continue;

        // Read the bit, shift the bits over by one and append the new bit.
        int bit = digitalRead(DATA);
        data = (data << 1) | bit;

        digitalWrite(CLK, LOW);
        delayMicroseconds(20);
    }

    digitalWrite(CLK, LOW);
    pinMode(DATA, OUTPUT);
    return data;
}