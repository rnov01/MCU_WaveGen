#include <stdio.h>
#include <string.h>
#include <math.h>

//creates line [time]ns   [voltage] for PWL file
void createLine(char *result, int timeStep, int voltage) {
    char temp[20];
    sprintf(temp, "%d", timeStep);
    strcat(result, temp);
    strcat(result, "ns\t");
    sprintf(temp, "%d", voltage);
    strcat(result, temp);
}
float T_ns(int freq) {
    float T = 1.0 / freq;
    printf("%f", T);
    return (T*(pow(10, 9)));
}
void main() {
    int tStep = 2, voltage = 5, T = 1000;
    char line[20];
    char temp[20];
    /*sprintf(temp, "%d", tStep);
    strcat(line, temp);
    strcat(line, "ns\t");
    sprintf(temp, "%d", voltage);
    strcat(line, temp);
    printf("STR: %s", line);*/
    T = (int)round(T_ns(10000));
    printf("T: %d", T);

}