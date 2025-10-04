#include <stdio.h>

void main() {
    char fileName[10]; 
    char content[] = "Hello";
    int bit = 1;
    FILE* fptr;
    sprintf(fileName, "bit_%d.txt", bit);
    fptr = fopen(fileName, "w");
    fprintf(fptr, content);
    fclose(fptr);
}