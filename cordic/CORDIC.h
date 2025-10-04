#include <stdio.h>
#ifndef CORDIC_H
#define CORDIC_H


float tAngle(float thetaOld, float sampleRate);

int cordic_loop(int ref_angle, int *x, int *y, int quad);

void cordic(unsigned long ref_angle, int *x, int *y);


#endif