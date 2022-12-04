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
#define PRINTFILE false

/* --- Inititial Global Variables --- */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t carGoes = PTHREAD_COND_INITIALIZER;

int timeInterval = 0;
bool rdyQRun = false;
bool carRun = true;

/* --- Time Struct --- */
struct clockManagement {
    int hour;
    int minute;
    int maxHour;
    int maxMinute;
    char cycle;
    char maxCycle;
    bool day;
} Hackett;

/* --- Main Struct --- */
struct broncoscountry {
    int WAITLINE;
    int CARNUM;
    int ARRIVED;
    int MAXPERCAR; 
    int REJECTED;
    int TOTALWHOSRIDE;
    int TOTALARRIVED;
    int TOTALREJECTED;
    int MAXLINEDUP;
    int TOTALWAITIME;
    int carsToThreadProd;
    int currentCar;
    int maxWait;
} Russ;

// "Default constructor". Set all the defaults for the variables
void set_defaults (int car_num, int max_per_car) {
    Russ.WAITLINE = 0;
    Russ.CARNUM = car_num;
    Russ.MAXPERCAR = max_per_car;
    Russ.TOTALARRIVED = 0;
    Russ.TOTALREJECTED = 0;
    Russ.TOTALWHOSRIDE = 0;
    Russ.MAXLINEDUP = 0;
    Russ.TOTALWAITIME = 0;
    Russ.carsToThreadProd = Russ.CARNUM * Russ.MAXPERCAR;
    Russ.currentCar = 1;
    Russ.maxWait = 0;

    Hackett.hour = 9;
    Hackett.minute = 0;
    Hackett.cycle = 'A';
    Hackett.day = true;
}

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
            Russ.REJECTED = pois;        
            Russ.TOTALREJECTED += pois;
        }
        // If the line exceeds capacity, figure out who to reject
        // by the following equation below
        else {
            rejection = (incoming + pois) - MAXWAITPEOPLE;
            Russ.REJECTED = rejection;
            Russ.TOTALREJECTED += rejection;
            incoming = MAXWAITPEOPLE;
        }
    }
    else {
        // Else, line is free, nobody is rejected
        incoming += pois;
        Russ.REJECTED = 0; 
    }

    return incoming;
} 

// Function to update the simulation clock with AM and PM
void update_clock() {
   int z = Hackett.minute;
   int hr = Hackett.hour;
   bool isDayTime = Hackett.day;

   if (z == 59) {
        hr++;
        z = 0;
        if (hr > 12) {
            hr = hr - 12; 
        }
        else if (hr == 12) {
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

// Find the time that has the FIRST longest waiting line
void find_max_wait() {
    int tempWaitLine = Russ.WAITLINE;
    int tempWhosRide = Russ.MAXLINEDUP;
    if (tempWaitLine > tempWhosRide) {
        Russ.MAXLINEDUP = Russ.WAITLINE;         
        Hackett.maxHour = Hackett.hour;
        Hackett.maxMinute = Hackett.minute;
        Hackett.maxCycle = Hackett.cycle;
    }
}

// Print function to file / to see outputs
void print_data(FILE *fp){
    if (PRINTFILE) {
        fprintf(fp,"%d,%d,%d,%d\n",timeInterval, Russ.ARRIVED, 
                Russ.REJECTED, Russ.WAITLINE); 
    }
    else {
        printf("%03d arrive %03d reject %03d wait-line %03d at %02d:%02d:00 %cM\n", 
                timeInterval, Russ.ARRIVED, Russ.REJECTED, Russ.WAITLINE, 
                Hackett.hour, Hackett.minute, Hackett.cycle);
    }
}

/* --- THREAD FUNCTIONS --- */
// This is the queue thread's function
void* ready_queue() {
    /* --- For outputing to file for graphs --- */
    char buffer[50];
    sprintf(buffer, "%dcars%dseats.csv", Russ.CARNUM, Russ.MAXPERCAR);
    char *filename = buffer;
    FILE *fp; 
    fp = fopen(filename,"w+");
    
    fprintf(fp,"time,arrived,rejected,waitline\n");

    int tempPois;
    int incoming;

    // For syncing the first interval. 
    rdyQRun = true;
    
    for (timeInterval = 0; timeInterval < MAXTIME; timeInterval++) {  
        /* --- Ready Queue's Critical Section --- */
        pthread_mutex_lock(&mutex);
        // To ensure if, at the begining, the cars start first, then they will
        // be stuck waiting. This is for broadcasting them to stop waiting. The queue will
        // in the cs before the cars so this will ensure the queue is ALWAYS first
        if (timeInterval == 0) {
            if (carRun) {
                pthread_cond_broadcast(&cond);
            }
        }

        /* Proccess of the ready queue */
        // Gets the people arrived for the waiting queue
        tempPois = poisson_return(timeInterval);
        Russ.ARRIVED = tempPois;
        Russ.TOTALARRIVED += tempPois;
        incoming = Russ.WAITLINE;

        // Determines if the queue is full, 
        // rejects people if necessary 
        Russ.WAITLINE = max_availability(incoming, tempPois);

        // Print for visualization / output
        print_data(fp);

        // Helper function to help find the worst spot in the queue
        find_max_wait();

        // Updating the time
        update_clock();
        
        // Tasked finished, braodcast all the threads
        pthread_cond_broadcast(&cond);

        // Call wait on queue so the cars can do its task
        pthread_cond_wait(&carGoes,&mutex); 

        pthread_mutex_unlock(&mutex);
    }

    pthread_cond_broadcast(&cond);

    return NULL;
}

// This is the car thread's functions
void* car() {
    // Synchronization, if a car thread goes first before
    // the queue, it will be forced to waiti. Mutex lock here
    // for when the other cars access the same values
    pthread_mutex_lock(&mutex);
    carRun = true;
    if (!rdyQRun) {
        pthread_cond_wait(&cond,&mutex);
    }
    pthread_mutex_unlock(&mutex);

    while(timeInterval < MAXTIME) {  
        /* --- Car's Critical Section --- */
        pthread_mutex_lock(&mutex);

        int tempWaitLine = Russ.WAITLINE;
        int tempMaxCar = Russ.MAXPERCAR;

        // Subtracts neccessary ammount of people from
        // the line
        if ((tempWaitLine - tempMaxCar) > 0) {
            Russ.WAITLINE -= tempMaxCar; 
            Russ.TOTALWHOSRIDE += tempMaxCar;
        }
        else {
            Russ.TOTALWHOSRIDE += tempWaitLine;
            Russ.WAITLINE = 0;
        }

        int atCar = Russ.currentCar;
        int maxCar = Russ.CARNUM;

        // Will signal the queue once on the last car.
        // Also updates the max waiting on the last car
        Russ.currentCar = atCar;
        if (atCar == maxCar) {
            Russ.maxWait += Russ.WAITLINE; 
            atCar = 1;
            pthread_cond_signal(&carGoes);
        }
        else {
            atCar++;
        }
        
        Russ.currentCar = atCar;
        pthread_cond_wait(&cond,&mutex); 
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

/* --- Helper Functions for MAIN --- */

// Creates the threads
void initiate_rides() {
    int car_num = Russ.CARNUM;
    int max_per_car = Russ.MAXPERCAR;

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
}

// Final announcement 
void final_announcement() {
    double avgWait = (double) Russ.maxWait / Russ.TOTALARRIVED;
    int avgMin = (int) avgWait;
    float avgSec = fmod(avgWait,(float) avgMin) * 60;
    printf(" ---- For %d Cars w/ %d Seats ---- \n", 
            Russ.CARNUM, Russ.MAXPERCAR); 
    printf("Total PPL Arrived:  %d\n", Russ.TOTALARRIVED); 
    printf("Total PPL Riding:   %d\n", Russ.TOTALWHOSRIDE); 
    printf("Total PPL Rejected: %d\n", Russ.TOTALREJECTED); 
    printf("Max People in Line: %d\n", Russ.MAXLINEDUP);
    printf("Longest Line Time:  %02d:%02d:00 %cM\n", Hackett.maxHour, 
            Hackett.maxMinute, Hackett.maxCycle);
    printf("Average Wait Time:  %.0f:%.0f minutes\n", avgWait, avgSec);
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

