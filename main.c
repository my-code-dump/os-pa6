#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <getopt.h>
#include <errno.h>
#include <stdbool.h>
#include "random437.h"
#define MAXWAITPEOPLE 20
#define MAXTIME 600
#define PRINTFILE false

/* --- Inititial Global Variables --- */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int timeInterval = 0;

struct totalWaitTime {
    int arrivals[MAXWAITPEOPLE];
    int qStart;
    int qEnd;
    int tStart;
    int tEnd;
    int onThread;
    int acc;
    bool wrapped;
} Berserk;

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
} Russ;

// Set all the defaults for the variables
void set_defaults (int car_num, int max_per_car) {
    Russ.WAITLINE = 0;
    Russ.CARNUM = car_num;
    Russ.MAXPERCAR = max_per_car;
    Russ.TOTALARRIVED = 0;
    Russ.TOTALREJECTED = 0;
    Russ.TOTALWHOSRIDE = 0;
    Russ.MAXLINEDUP = 0;
    Russ.TOTALWAITIME = 0;

    Hackett.hour = 9;
    Hackett.minute = 0;
    Hackett.cycle = 'A';
    Hackett.day = true;

    Berserk.qStart = 0;
    Berserk.qEnd = 0;
    Berserk.tStart = 0;
    Berserk.tEnd = 0;
    Berserk.onThread = 1;
    Berserk.acc = 0;
    Berserk.wrapped = false;
    for (int z = 0; z < MAXWAITPEOPLE; z++) {
        Berserk.arrivals[z] = 0; 
    } 

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

void find_total_wait_queue(int pois, int x) {
    int tempI = Berserk.qStart;
    int tempLim = Berserk.qEnd + pois;
    if (tempLim >= MAXWAITPEOPLE) {
        for (; tempI < MAXWAITPEOPLE; tempI++) {
            Berserk.arrivals[tempI] = x; 
        }
        tempI = 0;
        tempLim = tempLim - MAXWAITPEOPLE;
        for (; tempI < tempLim; tempI++) {
            Berserk.arrivals[tempI] = x; 
        }
        Berserk.wrapped = true;
    } 
    else {
        for (; tempI < tempLim; tempI++) {
            Berserk.arrivals[tempI] = x; 
        }
    }
    Berserk.qStart = tempI;
    Berserk.qEnd = tempLim;
}

void printArrThread(int y) {
    printf("t%d[", y);
    for (int i = 0; i < MAXWAITPEOPLE; i++) {
        printf("%2d ", Berserk.arrivals[i]);
    } 
    printf("]\n");
}

int placeHolder = 50;

void find_total_wait_car_smaller(int pois, int x, int pT) {
    int start = Berserk.tStart;
    int tempLim = start + Russ.MAXPERCAR;
    int tempACC = Berserk.acc;
    
    if (start >= (MAXWAITPEOPLE - 1)) {
        start = 0;
        tempLim = start + Russ.MAXPERCAR;
    }
    if (tempLim >= MAXWAITPEOPLE) {
        for (; start < MAXWAITPEOPLE; start++) {
            tempACC += x - Berserk.arrivals[start];
            Berserk.arrivals[start] = 0; 
        }
        start = 0;
        tempLim = tempLim - MAXWAITPEOPLE;
        for (; start < tempLim; start++) {
            tempACC += x - Berserk.arrivals[start];
            Berserk.arrivals[start] = 0; 
        }
        printArrThread(pT);
    } 
    else {
        for (; start < tempLim; start++) {
            tempACC += x - Berserk.arrivals[start];
            Berserk.arrivals[start] = 0; 
        }
        printArrThread(pT);
    }
    Berserk.tStart = start;
    Berserk.acc = tempACC;
}

void find_total_wait_cars_bigger(int pois, int x, int pT) {
    int start = Berserk.tStart;
    int tempLim = Berserk.qEnd;
    int seats = Russ.MAXPERCAR;
    int threadOffset = 0;
    int tempACC = Berserk.acc;
    bool remain = false;

    if (Berserk.wrapped) {
        if ((threadOffset + start + seats) < MAXWAITPEOPLE) {
            threadOffset += start + seats;
        } 
        else if ((threadOffset + start + seats) == MAXWAITPEOPLE){
            threadOffset = MAXWAITPEOPLE;
        } 
        else {
            threadOffset = MAXWAITPEOPLE;
            remain = true;
        }
    }
    else {
        if ((threadOffset + start + seats) < tempLim) {
            threadOffset += start + seats;
        } 
        else if ((threadOffset + start + seats) >= tempLim){
            threadOffset = Berserk.qEnd;
        } 
    }

    for (; start < threadOffset; start++) {
        tempACC += x - Berserk.arrivals[start];
        Berserk.arrivals[start] = 0;
    }

    printArrThread(pT);
    if (remain) {
        start = 0;
        threadOffset = 0;
        if ((threadOffset + start + seats) < tempLim) {
            threadOffset += start + seats;
        } 
        else if ((threadOffset + start + seats) >= tempLim){
            threadOffset = Berserk.qEnd;
        } 

        for (; start < threadOffset; start++) {
            tempACC += x - Berserk.arrivals[start];
            Berserk.arrivals[start] = 0;
        }
        Berserk.wrapped = false;
        printArrThread(pT);
    }
    //placeHolder++;
    Berserk.tStart = start;
    Berserk.acc = tempACC;
}

void printArr(int y) {
    printf("\n%d [", y);
    for (int i = 0; i < MAXWAITPEOPLE; i++) {
        printf("%2d ", Berserk.arrivals[i]);
    } 
    printf("]\n");
}

void testMe() {
    int ps = 10;
    int carthreadproduct = Russ.CARNUM * Russ.MAXPERCAR;
    for (int i = 0; i < MAXWAITPEOPLE; i++) {
        find_total_wait_queue(ps,(i + 1));
        printArr(1); 
        for (int z = 0; z < Russ.CARNUM; z++) {
            if (ps <= carthreadproduct) {
                find_total_wait_cars_bigger(ps,(i + 1),z);
            } 
            else {
                find_total_wait_car_smaller(ps,(i+1),z);
            }
        }
        printf("Acc = %d\n", Berserk.acc);
        printf("\n");
        //printArr(2); 
    }
}

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

    for (timeInterval = 0; timeInterval < MAXTIME; timeInterval++) {  
        pthread_mutex_lock(&mutex);
        /* Proccess of the ready queue */
        // Gets the people arrived for the waiting queue
        tempPois = poisson_return(timeInterval);
        Russ.ARRIVED = tempPois;
        Russ.TOTALARRIVED += tempPois;
        incoming = Russ.WAITLINE;

        // Determines if the queue is full, 
        // rejects people if necessary 
        Russ.WAITLINE = max_availability(incoming, tempPois);
        print_data(fp);
//        find_total_wait_queue(tempPois);
        find_max_wait();
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

        int tempWaitLine = Russ.WAITLINE;
        int tempMaxCar = Russ.MAXPERCAR;

        if ((tempWaitLine - tempMaxCar) > 0) {
            Russ.WAITLINE -= tempMaxCar; 
            Russ.TOTALWHOSRIDE += tempMaxCar;
        }
        else {
            Russ.TOTALWHOSRIDE += tempWaitLine;
            Russ.WAITLINE = 0;
        }

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

// Final announcement 
void final_announcement() {
    printf(" ---- For %d Cars w/ %d Seats ---- \n", 
            Russ.CARNUM, Russ.MAXPERCAR); 
    printf("Total PPL Arrived:  %d\n", Russ.TOTALARRIVED); 
    printf("Total PPL Riding:   %d\n", Russ.TOTALWHOSRIDE); 
    printf("Total PPL Rejected: %d\n", Russ.TOTALREJECTED); 
    printf("Max People in Line: %d\n", Russ.MAXLINEDUP);
    printf("Longest Line Time:  %02d:%02d:00 %cM\n", Hackett.maxHour, 
            Hackett.maxMinute, Hackett.maxCycle);
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
    set_defaults(2,4); 
    testMe();
//    initiate_rides();
  //  final_announcement();
    return 0;
}

