#ifndef JOYSTICK_H
#define JOYSTICK_H

extern const int JOY_LEFT;
extern const int JOY_RIGHT;
extern const int JOY_UP;
extern const int JOY_DOWN;

void joystickInit();
int joystickWaitForDir();
void joystickWaitForCenter();
int joystickZedDown();

#endif