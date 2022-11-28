#include "random437.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double U_Random() {
    return (double)rand()/RAND_MAX;
}

int poissonRandom(int meanArrival) {
    int invert = (-1) * meanArrival;
    int k = 0;
    long double p = 1.0;
    long double l = exp(invert);
    double u = U_Random();
    double F = l;
    
    while(u >= F) {   
        k++;
        l *= (double)meanArrival/k;
        F += l;
    }
    
    return k;
}

