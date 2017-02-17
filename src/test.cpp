#include <stdio.h>
#include "./uthread.h"

/**
 *  Declare a few functions for our threads to run... note that our current
 *  setup takes functions of the type:
 *    void (*)(void)
 */
void d(void* arg) { int i = 0; /* while (1) { if(i++ % 100 == 0)printf("in d: %d, %c, %d\n", i++, arg1, arg2);}*/ }
void e(void* c) { int i = 33; printf("in e %c\n", ((char*)c)[1]); while (1) { if(i++ % 100000 == 0)printf("in e: %d\n", i++); } }
void* f() { int i = 42; printf("in f\n"); while (i < 100000000) { if(i++ % 100000 == 0)printf("in f: %d\n", i++); } return &i; }
void* g() { char s = 'v'; thread_id* t = new thread_id;  printf("in g\n"); uthread_join(2,NULL); uthread_create(t, (void* (*)(void*))f, NULL); return &s;/*while (1) { if(i++ % 100000 == 0)printf("in g: %d\n", i++); }*/ }

int main()
{
    /*
    // for each thread, create a default context with the right stack
    for (unsigned i = 0; i < THREADS; ++i) {
        setjmp(context[i]);
        for(int k = 8; k < 13; k++)stack[(i) * 8192 + k] = 100;
        stack[i * 8192 + 4] = 'c';
        //stack[i * 8192 + 3] = 'd';
        ((unsigned*)context[i])[_SP] = xor_and_rol(&stack[(i) * 8192]);
    }

    // set each context's PC
    ((unsigned*)context[0])[_PC] = xor_and_rol((void*)d);
    ((unsigned*)context[1])[_PC] = xor_and_rol((void*)d);
    ((unsigned*)context[2])[_PC] = xor_and_rol((void*)d);
    ((unsigned*)context[3])[_PC] = xor_and_rol((void*)d);

    // switch to the first thread... note that this program will never return
    longjmp(context[0], 1);*/


    thread_id* t1 = new thread_id;
    char args[] = {'c','d','e','f','g','h','i','j'};
    //uthread_create(t1, (void* (*)(void*))d, args);

    uthread_create(t1, (void* (*)(void*))e, args);
    uthread_create(t1, (void* (*)(void*))f, NULL);
    uthread_create(t1, (void* (*)(void*))g, NULL);

    //uthread_create(t1, (void* (*)(void*))f, NULL);

    while(1){};
}
