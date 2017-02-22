/*************************************************************************
	> File Name: uthread.h
	> Author: Zhanhao Chen
	> Mail: zhc416@lehigh.edu
	> Created Time: Thu 16 Feb 2017 05:52:41 PM EST
 ************************************************************************/

#ifndef _UTHREAD_H
#define _UTHREAD_H

#define thread_id unsigned
#define uthread_mutex_t unsigned

#define THREAD_T thread_id
#define THREAD_CREATE uthread_create
#define THREAD_JOIN uthread_join
#define UTHREAD_MUTEX_INITIALIZER *((uthread_mutex_t*)malloc(sizeof(uthread_mutex_t)))
#define UTHREAD_COND_INITIALIZER *((uthread_cond_t*)malloc(sizeof(uthread_cond_t)))


int uthread_create(thread_id* t, void* (*start)(void*), void* args);
int uthread_join(thread_id t, void** status);
void uthread_exit(void* val_ptr);
int uthread_self();
void uthread_yield();

int uthread_mutex_lock(uthread_mutex_t *mutex);
int uthread_mutex_trylock(uthread_mutex_t *mutex);
int uthread_mutex_unlock(uthread_mutex_t *mutex);

typedef struct{
    int size;
    int begin;
    int tail;
    thread_id* followee;
} uthread_cond_t;

int uthread_cond_wait (uthread_cond_t* cond, uthread_mutex_t* mutex);
int uthread_cond_broadcast (uthread_cond_t* cond);
int uthread_cond_signal (uthread_cond_t* cond);

#endif
