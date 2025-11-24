#ifndef LED_BAR_H
#define LED_BAR_H

extern const unsigned char LED_ON;
extern const unsigned char LED_OFF;
extern const unsigned char LED_HALF;

void ledBarInit();
void ledBarClear();
void ledBarRefresh();
void ledBarSet(int led, unsigned char value);

#endif