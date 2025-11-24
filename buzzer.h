#ifndef BUZZER_H
#define BUZZER_H

void buzInit();
void buzPlay(int tone, int duration);
void buzPlayCountdown();
void buzPlaySuccess();
void buzPlayIncorrect();

#endif