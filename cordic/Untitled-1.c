#include <Arduino.h>

void main() {
    int *DDRC = 0x07;
    int *PORTC = 0x08;
    *DDRC = 0x00;
    while (1) {
        *PORTC ^= 0x01;
        delay_ms(500);
    }
}