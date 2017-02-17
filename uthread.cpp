/**
 *  Simple User-Level Threading Library Demonstration Code
 *
 *  This code is intended as a demonstration of a few of the more low-level
 *  issues involved in using setjmp/longjmp as the basis for a user-level
 *  thread library.  A jmp_buf struct is used to hold the thread context.
 *
 *  This code is very far from what the assignment requires.  You should think
 *  of it not even as a starting point, just as a reference.
 *
 *  Code tested on Ubuntu 10.4, gcc 4.4.5, -O0 optimization level... -O1 or
 *  higher will lead to run-time failures, for known reasons outside the scope
 *  of this project
 */

#include <unistd.h>
#include <cstdlib>
#include <setjmp.h>
#include <cstdio>
#include <signal.h>
#include <stdlib.h>

#include <signal.h>
#include <string.h>
#include <sys/time.h>

#include "uthread.h"

/**
 *  Defines for interacting with the thread context struct
 *
 *  NB: _BP would be 3
 */
#define _SP 4
#define _PC 5

#define COND_BLOCK 20

/**
 *  Linux hides addresses in jump buffers by xoring them with gs:0x18 and then
 *  rotating them left 9.  This code does the same, so that we can put our own
 *  addresses into a jump buffer
 *
 *  NB: gcc inline assembly "=r" indicates an in/out parameter
 */
int xor_and_rol(void* p)
{
    int ret = (int)p;
#ifndef __sun__
#warning "Linux"
    asm volatile ("xorl %%gs:0x18, %0;\n roll $0x9, %0;" : "=r"(ret));
#endif
    return ret;
}

/**
 *  This is just enough to get our threads working with a simple round-robin
 *  scheduler
 */
const unsigned THREADS     = 100;         // total number of threads
volatile unsigned curr_thread;         // the current thread
char* stack;   // stack for each thread
jmp_buf* context;        // jump buffer for each thread
bool* flag;        // jump buffer for each thread
volatile unsigned used_thread;
bool initialized = false;
unsigned mutex_id = 0;
unsigned cond_id = 0;

void timer_handler (int signum)
{
    uthread_yield();
}

void initialize(){

    /* initialize context */
    context = new jmp_buf[THREADS];
    stack = new char[THREADS * 8192];
    flag = new bool[THREADS];
    used_thread = 0;

    /* start scheduler */
    /* Install timer_handler as the signal handler for SIGVTALRM. */
    struct sigaction sa;
    struct itimerval timer;
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &timer_handler;
    sa.sa_flags = SA_NODEFER;

    /* Configure the timer to expire after 250 msec... */
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 100;
    /* ... and every 250 msec after that. */
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 100;
    /* Start a virtual timer. It counts down whenever this process is
    *    executing. */
    setitimer (ITIMER_VIRTUAL, &timer, NULL);
    sigaction (SIGVTALRM, &sa, NULL);

    /* save main thread */
    setjmp(context[0]);
    flag[0] = true;

    initialized = true;
}

void* end(char* args){
    void* result = ((void* (*)(void*))&args[0])((void*)(*((int*)(stack + curr_thread * 8192 + 8))));
    uthread_exit(result);
    return result;
}

int uthread_create(thread_id* t, void* (*start)(void*), void* args){
    if(!initialized)initialize();

    if(used_thread == THREADS-1 || setjmp(context[used_thread + 1]) != 0){
        return -1;
    }
    used_thread++;
    int new_context = used_thread;

    memcpy(&stack[new_context * 8192 + 4], &start, sizeof start);
    memcpy(&stack[new_context * 8192 + 4 + sizeof start], &args, sizeof args);

    ((unsigned*)context[new_context])[_SP] = xor_and_rol(&stack[(new_context) * 8192]);
    ((unsigned*)context[new_context])[_PC] = xor_and_rol((void*)end);
    flag[new_context] = true;
    *t = new_context;

    return 0;
}

/**
 *  Switch among threads
*/
void uthread_yield()
{
    // wait briefly, so that we can read the output of our threads
    //usleep(1000000);

    // on a call to yield_me, this call to setjmp will result in a return value
    // of 0.  A zero return value means that we will follow on to the
    // 'curr_thread = ...' line, where we will change the current thread and
    // longjmp to that thread.  That will, in turn, put us back on this next
    // setjmp line, but with a different context/stack and a return value of
    // 403, which will lead to us returning to a different thread than called
    // this function
    int tem = setjmp(context[curr_thread]);
    if(tem != 0)return;

    curr_thread = (curr_thread + 1) % THREADS;
    while(!flag[curr_thread])curr_thread = (curr_thread + 1) % THREADS;

    //printf("%d\n",curr_thread);

    // switch to the next thread (round robin)
    longjmp(context[curr_thread], 403);
}

void uthread_exit(void* val_ptr){
    flag[curr_thread] = false;
    memcpy(&stack[curr_thread * 8192 + 12], &val_ptr, sizeof val_ptr);
    uthread_yield();
}

int uthread_join(thread_id t, void** status){
    if(!flag[t])return -1;
    while(flag[t])uthread_yield();
    if(status)*status = &stack[t * 8192 + 12];
    return 0;
}

int uthread_self(){
    return curr_thread;
}

uthread_mutex_t uthread_mutex_init(){
    uthread_mutex_t* mutex = (uthread_mutex_t*)malloc(sizeof(uthread_mutex_t));
    mutex->taken = false;
    mutex->id = mutex_id++;
    return *mutex;
}

int uthread_mutex_lock(uthread_mutex_t *mutex){
    while(mutex->taken);
    mutex->taken = true;
    mutex->owner = curr_thread;
    return 0;
}

int uthread_mutex_trylock(uthread_mutex_t *mutex){
    if(mutex->taken)return -1;
    mutex->taken = true;
    mutex->owner = curr_thread;
    return 0;
}

int uthread_mutex_unlock(uthread_mutex_t *mutex){
    if(mutex->owner != curr_thread)return -1;
    mutex->taken = false;
    return 0;
}

uthread_cond_t uthread_cond_init(){
    uthread_cond_t *cond = (uthread_cond_t*)malloc(sizeof(uthread_cond_t));
    cond->id = cond_id++;
    cond->current = 0;
    cond->size = COND_BLOCK;
    cond->followee = new thread_id[COND_BLOCK];
    return *cond;
}

int uthread_cond_signal (uthread_cond_t* cond){
    if(cond->current == 0)return -1;
    cond->followee[0] = cond->followee[cond->current - 1];
    cond->current--;
    return 0;
}

int uthread_cond_broadcast(uthread_cond_t* cond){
    cond->current = 0;
    return 0;
}

void uthread_cond_expand(uthread_cond_t *cond){
    if(cond->size == 0)cond->size = 4;
    thread_id *new_followee = new thread_id[2 * cond->size];
    memcpy(new_followee, cond->followee, cond->size * sizeof(thread_id));
    cond->followee = new_followee;
    cond->size *= 2;
}

int uthread_cond_wait(uthread_cond_t *cond, uthread_mutex_t *mutex){
    if(uthread_mutex_unlock(mutex) != 0)return -1;
    if(cond->current == cond->size)uthread_cond_expand(cond);
    cond->followee[cond->current++] = curr_thread;
    bool block = true;
    do{
        block = false;
        for(int i = 0; i < cond->current; i++){
            if(cond->followee[i] == curr_thread){
                block = true;
                break;
            }
        }
    }while(block);
    if(uthread_mutex_lock(mutex) != 0)return -2;
    return 0;
}
