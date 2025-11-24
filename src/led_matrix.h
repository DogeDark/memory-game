#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#define SIZE 8

extern int currentFrame[SIZE][SIZE];

extern const int BLANK[SIZE][SIZE];
extern const int ARROW_LEFT[SIZE][SIZE];
extern const int ARROW_RIGHT[SIZE][SIZE];
extern const int ARROW_UP[SIZE][SIZE];
extern const int ARROW_DOWN[SIZE][SIZE];
extern const int INCORRECT[SIZE][SIZE];

void ledMatrixInit();
void ledMatrixSetFrame(const int frame[8][8]);

#endif