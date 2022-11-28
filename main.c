#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include "random437.h"
#define MAXWAITPEOPLE 800

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int line = 0;

void* ready_queue() {
    int incoming = poissonRandom(25); 
    pthread_mutex_lock(&mutex);
    if (line <= MAXWAITPEOPLE) {
        line += incoming;
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

void* car(int MAXPERCAR) {
    pthread_mutex_lock(&mutex);
    if ((line - MAXPERCAR) >= 0) {
        line -= MAXPERCAR; 
    }
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main (int argc, char** argv) {
    int CARNUM = 4;
    int MAXPERCAR = 7;
    pthread_t tid[CARNUM + 1];
    pthread_create(&tid[0], NULL, ready_queue, NULL);

    for (int i = 1; i <= CARNUM; i++) {
        pthread_create(&tid[i], NULL, car(MAXPERCAR), (void *)&i);
    }

    for (int i = 0; i <= CARNUM; i++) {
        pthread_join(tid[i],NULL);
    }

    return 0;
}
