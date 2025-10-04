#include <stdio.h>
#include <math.h>
#include <stdint.h>

#define iterations 16
#define pi 4*atan(1)
#define scale (1 << 8)

//LUT of arctan values used to calculate angles
const float atanLUT[iterations] = {0.7853981633974483, 0.4636476090008061, 0.24497866312686414,
    0.12435499454676144, 0.06241880999595735, 0.031239833430268277,
    0.015623728620476831, 0.007812341060101111, 0.0039062301319669718,
    0.0019531225164788188, 0.0009765621895593195, 0.0004882812111948983,
    0.00024414062014936177, 0.00012207031189367021, 0.00006103515617420877,
    0.000030517578115526096};
    


int stepGen() {
	return 0;
}

//compute cos coefficient to scale final x and y values
float kCompute(int it) {
	int i = 0;
	float con = cos(atanLUT[i]);
	for (i = 1; i < it; i++){
		con = con * cos(atanLUT[i]);
	}
	return con;
}

void bprint(int number) {
	int mask = 0x80;
	while (mask >= 0x01) {
		if (number & mask) {
			printf("1");
		} else {
			printf("0");
		}
		
		if (mask == 0x10) {
			printf(" ");
		}
		mask = mask >> 1;
	}
}

//compute theta values as a function of time
float tAngle(float thetaOld, float sampleRate, float frequency) {
	const float pie = 4*atan(1);
	float thetaNew = fmod((thetaOld + 2*pi*frequency*sampleRate), (2*pi));
	return thetaNew;
}

//perform cordic iterations
float cordic_loop(float ref_angle, float *x, float *y, int quad) {
	const float k = kCompute(iterations);
	float cordic_angle = 0, x_new = k, y_new = 0;
	int i, dir;
	*x = k;
	*y = 0;
	for (i = 0; i < iterations; i++) {
		if (cordic_angle < ref_angle) {
			//add
			dir = 1;
			cordic_angle = cordic_angle + atanLUT[i];
		} else if (cordic_angle > ref_angle) {
			//subtract
			dir = -1;
			cordic_angle = cordic_angle - atanLUT[i];
		} else {
			dir = 0;
		}
		x_new = *x - (dir * (*y) * (1.0/pow(2, i)));
		y_new = *y + (dir * (*x) * (1.0/pow(2, i)));

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

//perform pre/post shifts to get target angle in Q1 as necessary
float cordic(float ref_angle, float *x, float *y) {
	//create quadrant boundary angles
	float a90 = pi/2.0, a180 = pi, a270 = (3*pi)/2.0;
	float cordic_angle;
	//if angle in Q1, no shift, perform cordic loop
	if (ref_angle >= 0 && ref_angle <= a90) {
		cordic_angle = cordic_loop(ref_angle, x, y, 1);
		return cordic_angle;
	} else if (ref_angle > a90 && ref_angle <= a180) {
		//if in Q2, pre shift CW by 90 deg 
		ref_angle = pi - ref_angle;
		//perform cordic loop and adjust final angle by 90 deg
		cordic_angle = cordic_loop(ref_angle, x, y, 2);
		return cordic_angle;
	} else if (ref_angle > a180 && ref_angle <= a270) {
		//if in Q3, pre shift CW by 180 deg
		ref_angle = ref_angle - pi;
		//perform cordic loop and adjust result by 180 deg
		cordic_angle = cordic_loop(ref_angle, x, y, 3);
		return cordic_angle;
	} else if (ref_angle > a270 && ref_angle < 2*pi) {
		//if in Q4, pre shift CW by 270 deg
		ref_angle = (2*pi) - ref_angle;
		//perform cordic loop and adjust result by 270 deg
		cordic_angle = cordic_loop(ref_angle, x, y, 4);
		return cordic_angle;
	}
}



void errorDetect(int timeStep, float theta_actual, float theta_expected, float x, float y) {
	float ref_x = cos(theta_actual), ref_y = sin(theta_actual);
	float theta_error, cos_error, sin_error;
	if (theta_expected == 0.0) {
		theta_error = fabs(theta_actual - theta_expected);
	} else {
		theta_error = (fabs(theta_actual-theta_expected/theta_expected))*100;
	}
	if (ref_x == 0.0) {
		cos_error = fabs(x - ref_x);
	} else {
		cos_error = (fabs(x-ref_x/ref_x))*100;
	}
	if (ref_y == 0.0) {
		sin_error = fabs(y - ref_y);
	} else {
		sin_error = (fabs(y-ref_y/ref_y))*100;
	}
	if (theta_error > 2 || cos_error > 2 || sin_error > 2) {
		printf("Error at t = %d:\nTheta error: %f%%, sin error: %f%%, cos error: %f%%", timeStep, theta_error, sin_error, cos_error);
	}
}
void bitSelect(int bit, int tStep, int number) {
	int mask;
	switch (bit) {
		case 0:
			mask = 0x01;
			break;
		case 1:
			mask = 0x02;
			break;
		case 2: 
			mask = 0x04;
			break;
		case 3:
			mask = 0x08;
			break;
		case 4: 
			mask = 0x10;
			break;
		case 5:
			mask = 0x20;
			break;
		case 6:
			mask = 0x40;
			break;
		case 7 :
			mask = 0x80;
			break;
	}
	if (mask&number) {
		printf("1");
	} else {
		printf("0");
	}
}

//generate values for PWL file for spice sim
// set time step to 1/2 desired period -> 10ns
//based on cordic sine wave generation of 100 samples/cycle
void PWL(int bit, uint8_t y_val, int timeStep) {
	int mask, voltage;
	switch (bit) {
		case 0:
			mask = 0x01;
			break;
		case 1:
			mask = 0x02;
			break;
		case 2: 
			mask = 0x04;
			break;
		case 3:
			mask = 0x08;
			break;
		case 4: 
			mask = 0x10;
			break;
		case 5:
			mask = 0x20;
			break;
		case 6:
			mask = 0x40;
			break;
		case 7 :
			mask = 0x80;
			break;
	}
	if (mask&y_val) {
		voltage = 5;
	} else {
		voltage = 0;
	}
	printf("%dns\t%d\n", timeStep*10, y_val);
}
void main() {
	const float k = kCompute(iterations);
	float theta = 0.0;
	float sampleRate = 0.0001;
	float freq = 100;
	int tStep;
	float x, y;
	float cAngle;
	//loop through time steps
	for (tStep = 0; tStep < 45; tStep++){
		//printf("time = %d, theta = %f, sin(%f) = %f\n", tStep, theta, theta, y);
		cAngle = cordic(theta, &x, &y);
		printf("y: %f\n", y);
		uint8_t output = (uint8_t)(((y*scale) + 255)/2);
		//errorDetect(tStep, theta, cAngle, x, y);
		//printf("After Step %d:\nCordic Angle: %f, Theta =  %f\nResult Coord: (%f, %f), Expected Coord: (%f, %f)\n", tStep, cAngle, theta, x, y, cos(theta), sin(theta));
		//printf("t = %d:\tTheta = %f\tSIN(Theta) = %f\n", tStep, theta, y*(1<<8));
	
		PWL(7, output, tStep);
		theta = tAngle(theta, sampleRate, freq);
	
	
		
	}
	printf("K: %f", k);
	printf("\n%d", 1 << 16);
}

