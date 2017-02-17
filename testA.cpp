#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "uthread.h"
#include "tgetElapsedTime.h"
#include <stdint.h>


/**
 * Forward declaration of our test functions
 */
void* testfunc1(void*);
void* testfunc2(void*);

/**
 *  Part A requires implementing support for true parallelism
 */
int main()
{
    // make room for a bunch of uthreads
    THREAD_T tid[32];
    uthread_mutex_t mutex = UTHREAD_MUTEX_INITIALIZER;

    // Test #1: how long does it take for one thread to run?
    uint64_t t1 = getElapsedTime();
    THREAD_CREATE(&tid[0], testfunc2, (void*)&mutex);
    THREAD_JOIN(tid[0], NULL);
    uint64_t t2 = getElapsedTime();
    printf("Time for 1 thread:  %lld\n", t2-t1);

    // Test #2: how long does it take for four threads to run?
    uint64_t t3 = getElapsedTime();
    int i;
    for (i = 0; i < 4; ++i) {
        THREAD_CREATE(&tid[i], testfunc2, (void*)&mutex);
    }
    for (i = 0; i < 4; ++i) {
        THREAD_JOIN(tid[i], NULL);
    }
    uint64_t t4 = getElapsedTime();
    printf("Time for 4 threads: %lld\n", t4-t3);
}

/**
 *  Increment a counter that is in a cache line on my stack
 */
void* testfunc1(void* params)
{
    int pad1[64];
    volatile int counter = 0;
    int pad2[64];
    int i;
    for (i = 0; i < 1024*1024*32; ++i){
        ++counter;
        //if(i%1000000 == 0)printf("kkk:%d\n",counter); 
    }
}

void* testfunc2(void* params){
    uthread_mutex_lock((uthread_mutex_t*)params);
    for(int i = 0; i < 1024*1024*32; i++);
    uthread_mutex_unlock((uthread_mutex_t*)params);

}

void* testfunc3(uthread_mutex_t* mutex, uthread_cond_t* cond){
    uthread_mutex_lock(mutex);
    for(int i = 0; i < 1024*1024*32; i++);

    
}
