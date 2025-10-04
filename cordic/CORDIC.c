#define scale (1 << 8) //2^8 for fixed point calculations
#define iterations 15
#define K 155
#define pi 804
#define pi_2 1608
#define fixed_360 360*scale
#define samples_per_cycle 100//number of steps per sine wave cycle

const int atanLUT[iterations] = {11520, 6801, 3593, 1824, 916, 458, 229, 115, 57, 28, 14, 7, 4, 2, 1};


float tAngle(float thetaOld, float sampleRate, float frequency) {
    //int scaled_SR = trunc(sampleRate*scale);
    //printf("scaled sr: %d", scaled_SR);
    float angleIncrement = 360.0 / samples_per_cycle;//(360*frequency*sampleRate);
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
	//int cordic_angle;
	if (ref_angle >= 0 && ref_angle <= a90) {
    //Q1
		cordic_loop(ref_angle, x, y, 1);
    //Serial.print(" Quadrant: 1 ");
	} else if (ref_angle > a90 && ref_angle <= a180) {
		//Q2
		ref_angle = a180 - ref_angle;
		cordic_loop(ref_angle, x, y, 2);
    //Serial.print(" Quadrant: 2 ");
	} else if (ref_angle > a180 && ref_angle <= a270) {
		//Q3
		ref_angle = ref_angle - a180;
		cordic_loop(ref_angle, x, y, 3);
    //Serial.print(" Quadrant: 3 ");
	} else if (ref_angle > a270 && ref_angle <= a360){
		//Q4
		ref_angle = a360 - ref_angle;
		cordic_loop(ref_angle, x, y, 4);
    //Serial.print(" Quadrant: 4 ");
	}
  //Serial.print("CORDIC Final Output: x = ");
  //Serial.print(*x);
  //Serial.print(", y = ");
  //Serial.println(*y);
}