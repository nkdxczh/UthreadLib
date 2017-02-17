#if defined(STM_OS_LINUX)
#  include <unistd.h>
#  include <stdio.h>
#  include <cstring>
#  include <pthread.h>
#  include <time.h>
#  include <stdint.h>

  /**
   *  sleep_ms simply wraps the POSIX usleep call.  Note that usleep expects a
   *  number of microseconds, not milliseconds
   */
  inline void sleep_ms(uint32_t ms) { usleep(ms*1000); }

  /**
   *  Yield the CPU vis pthread_yield()
   */
  inline void yield_cpu() { pthread_yield(); }

  /**
   *  The Linux clock_gettime is reasonably fast, has good resolution, and is not
   *  affected by TurboBoost.  Using MONOTONIC_RAW also means that the timer is
   *  not subject to NTP adjustments, which is preferably since an adjustment in
   *  mid-experiment could produce some funky results.
   */
  inline uint64_t getElapsedTime()
  {
      struct timespec t;
      clock_gettime(CLOCK_REALTIME, &t);
      uint64_t tt = (((long long)t.tv_sec) * 1000000000L) + ((long long)t.tv_nsec);
      return tt;
  }

#endif // STM_OS_LINUX