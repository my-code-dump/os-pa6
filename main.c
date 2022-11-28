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

int i;
struct broncoscountry {
    int WAITLINE;
    int CARNUM;
    int ARRIVED;
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

int max_availability(int incoming, int time) {
    int pois = RUSS.ARRIVED;
    if ((incoming + pois) >= MAXWAITPEOPLE) {
        if (incoming == MAXWAITPEOPLE) {
            RUSS.REJECTED = pois;        
        }
        else {
            RUSS.REJECTED = (incoming + pois) - MAXWAITPEOPLE; 
            incoming = MAXWAITPEOPLE;
        }
    }
    else {
        incoming += pois;
    }

    return incoming;
} 

void print_data(int i){
    printf("%d arrive %d reject %d wait-line %d at \n", i, RUSS.ARRIVED, RUSS.REJECTED
            , RUSS.WAITLINE);
}

void* ready_queue() {
    int incoming;
    // I represents the time in minutes
    int pois;
    for (i = 0; i < MAXTIME; i++) {  
        pthread_mutex_lock(&mutex);
        
        /* Proccess of the ready queue */
        // Gets the people arrived for the waiting queue
        RUSS.ARRIVED = poisson_return(i);
        incoming = RUSS.WAITLINE;
        // Determines if the queue is full, rejects people if necessary 
        RUSS.WAITLINE = max_availability(incoming,i);
        
        //pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
       
        // Signal the other threads
        pthread_cond_broadcast(&cond);
        print_data(i);
        usleep(1);
    }

    pthread_cond_broadcast(&cond);
    return NULL;
}

void* car() {
    int local_max;
    int queue;
    while(i < MAXTIME) {  
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond,&mutex); 
        
        local_max = RUSS.MAXPERCAR;
        queue = RUSS.WAITLINE;

        if ((queue - local_max) >= 0) {
            RUSS.WAITLINE -= local_max; 
        //    printf("Pulled people\n");
        }
        else {
            RUSS.WAITLINE = 0;
        }

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void set_defaults (int car_num, int max_per_car) {
    RUSS.WAITLINE = 0;
    RUSS.CARNUM = car_num;
    RUSS.MAXPERCAR = max_per_car;
}

int main () {
    // Set the DEFAULTS HERE, CARNUM, MAXPERCAR
    set_defaults(4,7); 

    int car_num = RUSS.CARNUM;
    int max_per_car = RUSS.MAXPERCAR;

    pthread_t tid[car_num + 1];
    //pthread_create(&tid[0], NULL, ready_queue, NULL);

    for (int i = 1; i <= car_num; i++) {
        pthread_create(&tid[i], NULL, car, NULL);
    }

    pthread_create(&tid[0], NULL, ready_queue, NULL);

    for (int i = 0; i <= car_num; i++) {
        pthread_join(tid[i],NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
