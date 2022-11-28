#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include "random437.h"
#define MAXWAITPEOPLE 800
#define MAXTIME 600

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

struct broncoscountry {
    int WAITLINE;
    int CARNUM;
    int MAXPERCAR; 
    int REJECTED;
} RUSS;

int poisson_return(int time) {
    int out;
    if ((time < 120) || ((time >= 480) && (time < 600))) {
        out = poissonRandom(25); 
    } 
    else if ((time >= 120) && (time < 240)) {
        out = poissonRandom(45); 
    }    
    else if ((time >= 240) && (time < 480)) {
        out = poissonRandom(35); 
    }
    return out;
}

void max_availability(int incoming, int pois, int time) {
        if ((incoming + pois) > MAXWAITPEOPLE) {
             
        }
        else {
            incoming += pois;
  
        }

} 

void* ready_queue() {
    int incoming;
    // I represents the time in minutes
    int i;
    int pois;
    for (i = 0; i < MAXTIME; i++) {  
        pthread_mutex_lock(&mutex);
        incoming = RUSS.WAITLINE;

        pois = poisson_return(i);
        max_availability(incoming,pois,i);

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void* car() {
    int i = 0;
    while (i < MAXTIME) { 
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond,&mutex); 

        pthread_mutex_unlock(&mutex);
        i++;
    }
    return NULL;
}

void set_defaults (int car_num, int max_per_car) {
    RUSS.WAITLINE = 0;
    RUSS.CARNUM = car_num;
    RUSS.MAXPERCAR = max_per_car;
}

int main () {
    set_defaults(4,7); 

    int car_num = RUSS.CARNUM;
    int max_per_car = RUSS.MAXPERCAR;

    pthread_t tid[car_num + 1];
    pthread_create(&tid[0], NULL, ready_queue, NULL);

    for (int i = 1; i <= car_num; i++) {
        pthread_create(&tid[i], NULL, car, NULL);
    }

    for (int i = 0; i <= car_num; i++) {
        pthread_join(tid[i],NULL);
    }
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
