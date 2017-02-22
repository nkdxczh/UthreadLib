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
void* produce(void*);
void* consume(void*);

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

    uthread_mutex_t mutex1 = UTHREAD_MUTEX_INITIALIZER;
    uthread_cond_t cond = UTHREAD_COND_INITIALIZER;
    void* params[] = {(void*)&mutex1, (void*)&cond};
    for(i = 0; i < 10; i++){
        THREAD_CREATE(&tid[i], consume, (void*)params);
    }
    THREAD_CREATE(&tid[10], produce, (void*)&cond);
    THREAD_JOIN(tid[10], NULL);
    for(i = 0; i < 10; i++){
        THREAD_JOIN(tid[i], NULL);
    }
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
    printf("thread %d lock\n",uthread_self());
    for(int i = 0; i < 1024*1023*32; i++);
    printf("thread %d unlock\n",uthread_self());
    uthread_mutex_unlock((uthread_mutex_t*)params);

}

void* produce(void* params){
    printf("create producer %d\n",uthread_self());
    for(int i = 0; i < 5; i++){
        uthread_cond_signal((uthread_cond_t*)params);
        printf("produce\n");
        uthread_yield();
        usleep(100000);
    }
    uthread_cond_broadcast((uthread_cond_t*)params);
    printf("free all\n");
}

void* consume(void* params){
    uthread_mutex_t* mutex = (uthread_mutex_t*)((void**)params)[0];
    uthread_cond_t* cond = (uthread_cond_t*)((void**)params)[1];
    printf("create consumer %d\n",uthread_self());
    uthread_mutex_lock(mutex);
    uthread_cond_wait(cond, mutex);
    usleep(10000);
    printf("Consumer %d free\n",uthread_self());
    uthread_mutex_unlock(mutex);
}
