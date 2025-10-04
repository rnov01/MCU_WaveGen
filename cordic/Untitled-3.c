
#include <stdint.h>
#include <math.h>
#include <stdio.h>

#define scale (1 << 8) //2^8 for fixed point calculations
#define iterations 15
#define K 155
#define pi 804
#define pi_2 1608
#define fixed_360 360*scale

const int atanLUT[iterations] = {11520, 6801, 3593, 1824, 916, 458, 229, 115, 57, 28, 14, 7, 4, 2, 1};


float tAngle(float thetaOld, float sampleRate, float frequency) {
    //int scaled_SR = trunc(sampleRate*scale);
    //printf("scaled sr: %d", scaled_SR);
    float angleIncrement = (360*frequency*sampleRate);
	//int thetaNew =  (int) (thetaOld + 2*pi*frequency*scaled_SR);
    float thetaNew = (thetaOld + angleIncrement);
    while (thetaNew >= 360) {
            thetaNew -= 360;
    }
    while (thetaNew < 0) {
        thetaNew += 360;
    }
    //printf("%d", thetaOld);
	return thetaNew;
}

int cordic_loop(int ref_angle, int *x, int *y, int quad) {
	int cordic_angle = 0, x_new, y_new;
	int i;
	*x = K;
	*y = 0;
	for (i = 0; i < iterations; i++) {
		if (cordic_angle < ref_angle) {
			//add
			x_new = *x - (*y >> i);
            y_new = *y + (*x >> i);
			cordic_angle = cordic_angle + atanLUT[i];
		} else if (cordic_angle > ref_angle) {
			//subtract
			x_new = *x + (*y >> i);
            y_new = *y - (*x >> i);
			cordic_angle = cordic_angle - atanLUT[i];
		} else {
			x_new = *x;
            y_new = *y;
		}
		*x = x_new;
		*y = y_new;
		//printf("cordic angle: %f\n(%f, %f)\n", cordic_angle, *x, *y);
	}
	//adjust sin, cos values for quadrant
	switch (quad) {
			case 1:
				break;
			case 2:
				*x = (-1)*(*x);
				break;
			case 3:
				*x = (-1)*(*x);
				*y = (-1)*(*y);
				break;
			case 4:
				*y = (-1)*(*y);
				break;
	}
	return cordic_angle;
}

void cordic(int ref_angle, int *x, int *y) {
	//create quadrant boundary angles
	int a90 = 90*scale, a180 = 180*scale, a270 = 270*scale, a360 = 360*scale;
	int cordic_angle;
	//if angle in Q1, no shift, perform cordic loop
	if (ref_angle >= 0 && ref_angle <= a90) {
		cordic_loop(ref_angle, x, y, 1);
	} else if (ref_angle > a90 && ref_angle <= a180) {
		//if in Q2, pre shift CW by 90 deg 
		ref_angle = a180 - ref_angle;
		//perform cordic loop and adjust final angle by 90 deg
		cordic_loop(ref_angle, x, y, 2);
	} else if (ref_angle > a180 && ref_angle <= a270) {
		//if in Q3, pre shift CW by 180 deg
		ref_angle = ref_angle - a180;
		//perform cordic loop and adjust result by 180 deg
		cordic_loop(ref_angle, x, y, 3);
	} else /*(ref_angle > a270 && ref_angle < 2*pi)*/{
		//if in Q4, pre shift CW by 270 deg
		ref_angle = a360 - ref_angle;
		//perform cordic loop and adjust result by 270 deg
		cordic_loop(ref_angle, x, y, 4);
	}
}

int main() {
	float theta = 0;
	float sampleRate = .0001;
	float freq = 10;
	int x, y;
    int i;
  //DDRD |= 0xFF;
  //DDRB |= 0x3F;
  //DDRC |= 0x03;

  /*while(1){
    cordic(theta, &x, &y);
    //PORTD = y;
    theta = tAngle(theta, sampleRate, freq);
    //delay(1);
    printf("%d\n", y);
  }*/

    for (i = 0; i < 1000 ; i++) {
        cordic(theta*scale, &x, &y);
        printf("t = %d:\tTheta = %f\tSIN(Theta) = %d\n", i, theta, y);
        theta = tAngle(theta, sampleRate, freq);
    }
  return 0;
}