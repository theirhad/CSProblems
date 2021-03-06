/*
-> Author: Semir Elezovikj
-> Date: September 7th, 2013

-> Purpose: 
The purpose of this program is to observe the cost of time slicng in the case
of running multithreaded programs. In order to achieve this, we have a function tfunc 
which performs a lengthy computation based on the number of iterations it receives as
an argument. 



-> Instructions for running the program

To run the program first compile it in the following manner: 

gcc -o project1 project1.c -lpthread


Once successfully compiled, the program can be run like this:
usage: ./project1 [nrThreads] [iterations]

nrThreads - number of threads 
iterations - number of iterations for function tfunc

Running the program with these parameters:

./project1 10 5000000

yields a result like the one below:

Number of threads        | Number of iterations     
10                       | 500000                   
Finished in 2.264 seconds.  
 */
#define _GNU_SOURCE
#include <sched.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>


void *tfunc(void *arg) {
    int n = (int)arg;
    int i;
    int j;
    volatile int x;
    for (i=0; i<n; i++) {
        /* this loop is the “work” that each thread does */
        x = 0;
        for (j=0; j<1000; j++) x = x+j;
    }
    return(0);
}

int assignToCore(int core_id, int threadId) {
   int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
   // printf("Number of cores: %d\n", num_cores);
   if (core_id >= num_cores) return EINVAL;
   
   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);
   CPU_SET(core_id, &cpuset);
   int ret = pthread_setaffinity_np(threadId, sizeof(cpu_set_t), &cpuset);;
   // printf("Thread ID [%d]\t affinity: [%d]\n", threadId, pthread_getaffinity_np(threadId, sizeof(cpu_set_t), &cpuset));
   return ret;
}

double markTime() {
    /* Specification for timeval
    struct timeval {
        // seconds
        time_t      tv_sec;     
        // microseconds
        suseconds_t tv_usec; 
    };
    */
    struct timeval mark;
    gettimeofday(&mark, NULL);
    return (double) mark.tv_sec + mark.tv_usec / 1000000.;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: ./project1 [nrThreads] [iterations]\n");
        return 1;
    }
    
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    printf("Number of cores: %d\n", num_cores);
    double elapsedTime;
    elapsedTime = markTime();

    // The first argument is the number of threads executing simultaneously
    int nrThreads = atoi(argv[1]);
    // number of iterations performed in tfunc for each thread
    int iterations = atoi(argv[2]) / nrThreads;
    pthread_t thread_id[nrThreads];
    int status, *p_status = &status;

    printf("%-25s| %-25s\n", "Number of threads", "Number of iterations");
    printf("%-25d| %-25d\n", nrThreads, iterations);
    int i = 0;
    for (i = 0; i < nrThreads; ++i) {    // generate threads
        // First argument: as each thread is created, its thread ID is saved in the thread_id array
        // Second argument: NULL - each created thread has the default set of attributes
        // Third argument: the function executed in the thread
        // Fourth: the argument passed to the function executed in the thread
        if( pthread_create(&thread_id[i], NULL, tfunc, (void*)iterations) > 0){	    
            printf("pthread_create failure with Thread Nr. %d\n", i);
            return 2;
        } else {
			if(i %2 == 0) assignToCore(0, thread_id[i]);
			else assignToCore(1, thread_id[i]);			
		}
    }
    
    for (i=0; i < nrThreads; ++i) {      // wait for each thread
        // After the creation of the threads, the program waits for the threads to finish
        if ( pthread_join(thread_id[i], (void **) p_status) > 0){
            printf("pthread_join failure with Thread Nr. %d\n", i);
            return 3;
        }        
        // printf("Thread Nr. %d, Thread ID: [%d] returns\n", i, thread_id[i]);
        
    }
    elapsedTime = markTime() - elapsedTime;
    printf("Finished in %.3f seconds. \n", elapsedTime);

    return 0;
}
