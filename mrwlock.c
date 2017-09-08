#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mrwlock.h"
#include <sched.h>

#define MAX_THREADS 64
typedef struct read_locks_s {
    unsigned int __reading __attribute__ ((aligned(32)));
    unsigned int __tid __attribute__ ((aligned(32)));
} read_locks_t;

read_locks_t read_locks[MAX_THREADS];
static __thread int th_idx = -1;

int mrwlock_init (mrwlock_t *mrwlock)
{
    memset(mrwlock, 0, sizeof(*mrwlock));
    return 0;
}

/* Get Read lock */
int mrwlock_rdlock (mrwlock_t *mrwlock)
{
    int i = 0;
    read_locks_t *reads;

    /* Obtain a thread index for this thread in the first pass */
    if (th_idx == -1) {
        for (i = 0; i < MAX_THREADS; i++) {
            if (read_locks[i].__tid == 0) {
                read_locks[i].__tid = pthread_self();
                th_idx = i;
                break;
            }
        }
    }

    if (read_locks[th_idx].__reading)
        return 0;

    while (1) {
        /* Get the rwlock if there is no writer...  */
        if (mrwlock->__writer == 0) {
            read_locks[th_idx].__reading = 1;
            __sync_synchronize();
            if (mrwlock->__writer == 0) {
                return 0;
            } else {
                read_locks[th_idx].__reading = 0;
            }
        }

        sched_yield();
        __sync_synchronize();
    }

    return 0;
}

/* Get Write Lock */
int mrwlock_wrlock (mrwlock_t *mrwlock)
{
  int i = 0;

  __sync_synchronize();

  if (mrwlock->__writer == pthread_self())
    return 0;

      if (mrwlock->__writer == 0)
        {
          /* Mark self as writer.  */
          /* If there were more than 1 writer, consider using
             __sync_bool_compare_and_swap to change the writer */
          mrwlock->__writer = pthread_self();

          /* Now that we have the writer, wait for all reads to complete */
          for (i = 0; i < MAX_THREADS; i++)
            {
              if (read_locks[i].__tid)
                {
                  if (read_locks[i].__reading)
                      sched_yield();
                }
             }
          }

    return 0;
}


/* Unlock MRWLOCK.  */
int mrwlock_unlock (mrwlock_t *mrwlock)
{
    /* Yield if writer to let readers get some CPU time */
    if (mrwlock->__writer == pthread_self()) {
        mrwlock->__writer = 0;
        __sync_synchronize();
        sched_yield();
    } else
        read_locks[th_idx].__reading = 0;

    return 0;
}
