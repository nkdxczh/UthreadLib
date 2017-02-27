# UthreadLib

##Thread Construction
###Data Structure:
thread_id: unsigned <br />
###Functions:
uthread_create: create a new thread end by call uthread_exit<br />
uthread_yeid: switch to next context. called by timer.<br />
uthread_exit: modify alive flag and save return pointer into stack.<br />
uthread_join: block and check target thread's alive flag. return the pointer from thread stack.<br />

##Mutex Lock
###Data Structure:
uthread_mutex_t: unsigned <br />
###Functions:
uthread_mutex_lock: use __sync_bool_compare_and_swap to block current thread and yeid if fail to swap.<br />
uthread_mutex_lock: use __sync_bool_compare_and_swap try to get lock. return -1 if fail<br />
uthread_mutex_unlock: use reversed __sync_bool_compare_and_swap to release the lock<br />

##Cond
###Data Structure:
uthread_cond_t: 
strcut{
  int size;
  int begin;
  int tail;
  thread_id* followee;
}
###Functions:
uthread_cond_wait: release a mutex lock and wait for cond's signal.<br />
uthread_cond_signal: release a thread that is waiting for this cond.<br />
uthread_cond_broadcast: release all threads that are waiting for this cond.<br />

##Make & Run

Make: make in root folder

Run: ./run.sh or run testA in obj folder
