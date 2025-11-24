/*

The buzzer is PWM controlled. We use softTone to handle the PWM.

*/

#include <wiringPi.h>
#include <softTone.h>

#define BUZZER 26

// Initialize the buzzer peripheral.
void buzInit()
{
    softToneCreate(BUZZER);
}

void buzPlay(int tone, int duration)
{
    softToneWrite(BUZZER, tone);
    delay(duration);
    softToneWrite(BUZZER, 0);
}

void buzPlayCountdown()
{
    softToneWrite(BUZZER, 784);
    delay(120);
    softToneWrite(BUZZER, 0);
    delay(480);
    softToneWrite(BUZZER, 784);
    delay(120);
    softToneWrite(BUZZER, 0);
    delay(480);
    softToneWrite(BUZZER, 784);
    delay(480);
    softToneWrite(BUZZER, 0);
}

void buzPlaySuccess()
{
    softToneWrite(BUZZER, 659);
    delay(220);
    softToneWrite(BUZZER, 523);
    delay(220);
    softToneWrite(BUZZER, 784);
    delay(300);
    softToneWrite(BUZZER, 0);
}

void buzPlayIncorrect()
{
    for (int i = 0; i < 3; i++)
    {
        softToneWrite(BUZZER, 120);
        delay(220);
        softToneWrite(BUZZER, 0);
        delay(220);
    }
}