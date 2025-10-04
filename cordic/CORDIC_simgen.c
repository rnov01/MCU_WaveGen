#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "CORDIC.h"

#define scale (1 << 8) //2^8 for fixed point calculations
#define iterations 15
#define K 155
#define fixed_360 360*scale
#define samples_per_cycle 100//number of steps per sine wave cycle

//const int atanLUT[iterations] = {11520, 6801, 3593, 1824, 916, 458, 229, 115, 57, 28, 14, 7, 4, 2, 1};


/*float tAngle(float thetaOld, float sampleRate) {
    float angleIncrement = 360.0 / samples_per_cycle;
    float thetaNew = (thetaOld + angleIncrement);
    while (thetaNew >= 360) {
            thetaNew -= 360;
    }
    while (thetaNew < 0) {
        thetaNew += 360;
    }
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
	}
	//adjust sin, cos values for quadrant
	switch (quad) {
			case 1:
				break;
			case 2:
				*x = -(*x);
				break;
			case 3:
				*x = -(*x);
				*y = -(*y);
				break;
			case 4:
				*y = -(*y);
				break;
	}
	return cordic_angle;
}

void cordic(unsigned long ref_angle, int *x, int *y) {
	//create quadrant boundary angles
	unsigned long a90 = 90UL * scale, a180 = 180UL * scale, a270 = 270UL * scale, a360 = 360UL * scale;
	if (ref_angle >= 0 && ref_angle <= a90) {
    //Q1
		cordic_loop(ref_angle, x, y, 1);
	} else if (ref_angle > a90 && ref_angle <= a180) {
		//Q2
		ref_angle = a180 - ref_angle;
		cordic_loop(ref_angle, x, y, 2);
	} else if (ref_angle > a180 && ref_angle <= a270) {
		//Q3
		ref_angle = ref_angle - a180;
		cordic_loop(ref_angle, x, y, 3);
	} else if (ref_angle > a270 && ref_angle <= a360){
		//Q4
		ref_angle = a360 - ref_angle;
		cordic_loop(ref_angle, x, y, 4);
	}
}*/

//check if file exists
void fileCheck(FILE *ptr, bool fileExist) {
	if (ptr != NULL) {
		return true;
	} else {
		return false; 
	}
}

void getParameters(int *freq){
    int usr_val;
    printf("Please enter the desired frequency:\n");
    scanf("%d", &usr_val);
    *freq = usr_val;
}

//sets all array characters to '\0'
void clearArray(char *array) {
    memset(array, 0, 30);
}

//creates line [time]ns   [voltage] for PWL file
void writeLine(FILE* ptr, char *result, int timeStep, int voltage) {
    char temp[20];
    clearArray(result);
    sprintf(temp, "%d", timeStep);
    strcat(result, temp);
    clearArray(temp);
    strcat(result, "ns\t");
    sprintf(temp, "%d\n", voltage);
    strcat(result, temp);
    clearArray(temp);
    fprintf(ptr, "%s", result);
}

//returns period of input frequency in nanoseconds
float T_ns(int freq) {
    float T = 1.0 / freq;
    return T*(pow(10, 9));
}

//generate values for PWL file for spice sim
int PWL(FILE* ptr, int bit, uint8_t y_val, int timeStep, int vLast, char *line, int freq) {
	int mask, voltage, riseTime = 1;
    char result[30];
    int tStep_multiplier = (int)round(T_ns(freq) / 100.0);
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
	if (mask & y_val) {
		voltage = 5;
	} else {
		voltage = 0;
	}
    if (voltage == vLast) {
	    //printf("%dns\t%d\n", timeStep*1000, voltage);
        writeLine(ptr, result, timeStep*tStep_multiplier, voltage);
        //printf("RESULT: %s\n", result);
        //printf("mult: %d\n", tStep_multiplier);

    } else {
        //printf("%dns\t%d\n", (timeStep*1000)-riseTime, vLast);
        if (timeStep != 0) {
            writeLine(ptr, result, (timeStep*tStep_multiplier)-riseTime, vLast);
            //printf("RESULT: %s\n", result);
            //printf("%dns\t%d\n", timeStep*1000, voltage);
            writeLine(ptr, result, timeStep*tStep_multiplier, voltage);
            //printf("RESULT: %s\n", result);
        }
    } 
    return voltage;
}


void main() {
    static float theta = 0;
    float sampleRate = 0.001;
    static unsigned long lastTime = 0;
    int x, y;
    int tStep,vLast = 0, bit;
    char line[100];
    char fileName[10];
    //frequency in Hz
    int frequency = 12000;
    if (frequency < 1000){
        sampleRate = 0.0001;
    } else {
        sampleRate = 0.001;
    }
    /*do {
        printf("Enter frequency: ");
        scanf(" %d", &frequency);
    } while (frequency <= 0);*/

    FILE* fptr;
    for (bit=0;bit<8;bit++) {
        sprintf(fileName, "bit_%d.txt", bit);
        fptr = fopen(fileName, "w");
        //printf("FILENAME: %s", fileName);
        for (tStep = 0; tStep < 100; tStep++) {
            cordic((unsigned long)(theta*scale), &x, &y);
            uint8_t output = (uint8_t)((y + 255)/2);
            vLast = PWL(fptr, bit, output, tStep, vLast, line, frequency);   
            theta = tAngle(theta, sampleRate);
        }
        fclose(fptr);
        printf("%s written succesfully...\n", fileName);
    }    
}
