#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <getopt.h>
#include <errno.h>
#include <stdbool.h>
#include "random437.h"
#define MAXWAITPEOPLE 800
#define MAXTIME 600
#define PRINTFILE true

/* --- Inititial Global Variables --- */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int timeInterval = 0;

struct clockManagement {
    int hour;
    int minute;
    bool day;
    char cycle;
} Hackett;

/* --- Main Struct --- */
struct broncoscountry {
    int WAITLINE;
    int CARNUM;
    int ARRIVED;
    int MAXPERCAR; 
    int REJECTED;
    int TOTALARRIVED;
    int TOTALWHOSRIDE;
    int TOTALREJECTED;
    int arrival[MAXTIME];
} RUSS;

/* --- Helper Functions for Threads --- */
// This function helps get the poisson number through the timeslot
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

// This function helps determine who gets to stay in line and whos 
// rejected
int max_availability(int incoming, int pois) {
    int rejection;
    if ((incoming + pois) >= MAXWAITPEOPLE) {
        // If line at the capacity, the incoming people are rejected
        if (incoming == MAXWAITPEOPLE) {
            RUSS.REJECTED = pois;        
            RUSS.TOTALREJECTED += pois;
        }
        // If the line exceeds capacity, figure out who to reject
        // by the following equation below
        else {
            rejection = (incoming + pois) - MAXWAITPEOPLE;
            RUSS.REJECTED = rejection;
            RUSS.TOTALREJECTED += rejection;
            incoming = MAXWAITPEOPLE;
        }
    }
    else {
        // Else, line is free, nobody is rejected
        incoming += pois;
        RUSS.REJECTED = 0; 
    }

    return incoming;
} 

void update_clock() {
   int z = Hackett.minute;
   int hr = Hackett.hour;
   bool isDayTime = Hackett.day;
   if (z == 59) {
        hr++;
        z = 0;
        if (hr > 12) {
            hr = hr - 12; 
            if (isDayTime) {
                isDayTime = false;
            }
            else {
                isDayTime = true;
            }
        }
   }
   else {
       z++;
   }

   if (isDayTime) {
        Hackett.cycle = 'A';
   }
   else {
        Hackett.cycle = 'P';
   }

   Hackett.hour = hr;
   Hackett.minute = z;
   Hackett.day = isDayTime;

}

void print_data(FILE *fp){
    if (PRINTFILE) {
        fprintf(fp,"%d,%d,%d,%d\n",timeInterval, RUSS.ARRIVED, 
                RUSS.REJECTED, RUSS.WAITLINE); 
    }
    else {
        printf("%03d arrive %03d reject %03d wait-line %03d at %02d:%02d:00 %cM\n", 
                timeInterval, RUSS.ARRIVED, RUSS.REJECTED, RUSS.WAITLINE, 
                Hackett.hour, Hackett.minute, Hackett.cycle);
    }
}

/* --- THREAD FUNCTIONS --- */
// This is the queue thread's function
void* ready_queue() {
    /* --- For outputing to file for graphs --- */
    char buffer[50];
    sprintf(buffer, "%dcars%dseats.csv", RUSS.CARNUM, RUSS.MAXPERCAR);
    char *filename = buffer;
    FILE *fp; 
    fp = fopen(filename,"w+");
    
    fprintf(fp,"time,arrived,rejected,waitline\n");

    int tempPois;
    int incoming;
    // I represents the time in minutes
    for (timeInterval = 0; timeInterval < MAXTIME; timeInterval++) {  
        pthread_mutex_lock(&mutex);
        /* Proccess of the ready queue */
        // Gets the people arrived for the waiting queue
        tempPois = poisson_return(timeInterval);
        RUSS.ARRIVED = tempPois;
        RUSS.TOTALARRIVED += tempPois;
        incoming = RUSS.WAITLINE;

        // Determines if the queue is full, 
        // rejects people if necessary 
        RUSS.WAITLINE = max_availability(incoming, tempPois);
        print_data(fp);
        update_clock();
        
        //pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
       
        // Signal the other threads
        pthread_cond_broadcast(&cond);
        
        // This sleeps allows enough time for 
        // every other proccess to finish 
        usleep(1000);
    }

    pthread_cond_broadcast(&cond);
    usleep(1000);
    return NULL;
}

// This is the car thread's functions
void* car() {
    while(timeInterval < MAXTIME) {  
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond,&mutex); 

        int tempWaitLine = RUSS.WAITLINE;
        int tempMaxCar = RUSS.MAXPERCAR;

        if ((tempWaitLine - tempMaxCar) > 0) {
            RUSS.WAITLINE -= tempMaxCar; 
            RUSS.TOTALWHOSRIDE += tempMaxCar;
        }
        else {
            RUSS.TOTALWHOSRIDE += tempWaitLine;
            RUSS.WAITLINE = 0;
        }

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

/* --- Helper Functions for MAIN --- */
void set_defaults (int car_num, int max_per_car) {
    RUSS.WAITLINE = 0;
    RUSS.CARNUM = car_num;
    RUSS.MAXPERCAR = max_per_car;
    RUSS.TOTALARRIVED = 0;
    RUSS.TOTALREJECTED = 0;
    RUSS.TOTALWHOSRIDE = 0;
    Hackett.hour = 9;
    Hackett.minute = 0;
    Hackett.cycle = 'A';
    Hackett.day = true;
}

void initiate_rides() {
    int car_num = RUSS.CARNUM;
    int max_per_car = RUSS.MAXPERCAR;

    pthread_t tid[car_num + 1];

    for (int i = 1; i <= car_num; i++) {
        pthread_create(&tid[i], NULL, car, NULL);
    }

    pthread_create(&tid[0], NULL, ready_queue, NULL);

    for (int i = 0; i <= car_num; i++) {
        pthread_join(tid[i],NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}

void final_announcement() {
    printf(" ---- For %d Cars w/ %d Seats ---- \n", 
            RUSS.CARNUM, RUSS.MAXPERCAR); 
    printf("Total PPL Arrived:  %d\n", RUSS.TOTALARRIVED); 
    printf("Total PPL Riding:   %d\n", RUSS.TOTALWHOSRIDE); 
    printf("Total PPL Rejected: %d\n", RUSS.TOTALREJECTED); 

}

/* --- MAIN --- */
int main (int argc, char** argv) {
    int n = 0;
    int m = 0;
    int opt = 0;

    while ((opt = getopt(argc,argv,"N:M:")) != -1) {
        switch (opt) {
            case 'N':
                n = atoi(optarg);
                break;
            case 'M':
                m = atoi(optarg);
                break;
            default:
                printf("Invalid Inputs\n");
                break;
        }
    }
    // Set the DEFAULTS HERE, CARNUM, MAXPERCAR
    set_defaults(n,m); 
    initiate_rides();
    final_announcement();
    return 0;
}
