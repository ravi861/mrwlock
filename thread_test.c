#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdbool.h>
#include "mrwlock.h"

FILE *fp;
#define LEN 64
int arr[LEN];
int total_write_ops = 0;
int total_read_ops = 0;
#define RUNTIME 10

#define TCOUNT 32

pthread_mutex_t ready_mutex = PTHREAD_MUTEX_INITIALIZER;
mrwlock_t mrwlock;
pthread_rwlock_t rw_mutex;

struct thread_info {            /* Used as argument to thread_start() */
    pthread_t thread_id;        /* ID returned by pthread_create() */
    int       thread_num;       /* Application-defined thread # */
}thread_info;

void* worker_thread(void *arg)
{
    int i = 0, val = 0;
    struct thread_info *tinfo = arg;
    int ops = 0;
    struct timespec start, finish;
    clock_gettime(CLOCK_MONOTONIC, &start);

    fprintf(fp, "Starting thread %d\n", tinfo->thread_num);
    while (1) {
#ifdef MRW
        mrwlock_rdlock(&mrwlock);
#elif RW
        pthread_rwlock_rdlock(&rw_mutex);
#else
        pthread_mutex_lock(&ready_mutex);
#endif
        for (i = 0; i < LEN; i++) { val = arr[i]; }
        total_read_ops++;
#ifdef MRW
        mrwlock_unlock(&mrwlock);
#elif RW
        pthread_rwlock_unlock(&rw_mutex);
#else
        pthread_mutex_unlock(&ready_mutex);
#endif

        clock_gettime(CLOCK_MONOTONIC, &finish);
        ops++;
        if ((finish.tv_sec - start.tv_sec) >= RUNTIME)
            break;
    };
    fprintf(fp, "Total ops for thread %d is %d\n", tinfo->thread_num, ops);

    return NULL;
}

void* write_thread(void *arg)
{
    int i = 0;
    struct thread_info *tinfo = arg;
    int ops = 0;
    struct timespec start, finish;
    clock_gettime(CLOCK_MONOTONIC, &start);

    fprintf(fp, "Starting thread %d\n", tinfo->thread_num);
    while (1) {
#ifdef MRW
        mrwlock_wrlock(&mrwlock);
#elif RW
        pthread_rwlock_wrlock(&rw_mutex);
#else
        pthread_mutex_lock(&ready_mutex);
#endif
        for (i = 0; i < LEN; i++) { arr[i] = arr[i] * 2; }
        total_write_ops++;
#ifdef MRW
        mrwlock_unlock(&mrwlock);
#elif RW
        pthread_rwlock_unlock(&rw_mutex);
#else
        pthread_mutex_unlock(&ready_mutex);
#endif
        clock_gettime(CLOCK_MONOTONIC, &finish);
        ops++;
        if ((finish.tv_sec - start.tv_sec) >= RUNTIME)
            break;
    };
    fprintf(fp, "Total write ops for thread %d is %d\n", tinfo->thread_num, ops);

    return NULL;
}

int main(int argc, char *argv[])
{
    int i, err, s;
    struct thread_info *threads = calloc(TCOUNT + 1, sizeof(struct thread_info));
    char lineval[80], line[80];
    void *res;
    void *test = NULL;


    fp = fopen("output.log", "w");
    for (i = 0; i < LEN; i++) { arr[i] = i; }

#ifdef MRW
    printf("MRW Locks used\n");
    mrwlock_init(&mrwlock);
#elif RW
    printf("RW Locks used\n");
    pthread_rwlockattr_t rwlock_attr;
    pthread_rwlockattr_init(&rwlock_attr);
    pthread_rwlockattr_setkind_np(&rwlock_attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
    /* Set the attr argument as NULL if you don't care about writes at all */
    pthread_rwlock_init(&rw_mutex, &rwlock_attr);
#else
    printf("Mutex locks used\n");
#endif

    /* Start the writer thread */
    threads[TCOUNT].thread_num = TCOUNT;
    pthread_create(&threads[TCOUNT].thread_id, NULL, write_thread, &threads[TCOUNT]);

    /* Kick off the reader threads */
    for (i = 0; i < TCOUNT; i++) {
        threads[i].thread_num = i;
        err = pthread_create(&threads[i].thread_id, NULL, worker_thread, &threads[i]);
    }

    for (i = 0; i <= TCOUNT; i++) {
        s = pthread_join(threads[i].thread_id, &res);
        free(res);
    }

    fprintf(fp, "Total Ops; Read %d Write %d\n", total_read_ops, total_write_ops);
    fclose(fp);

    free(threads);
    exit(0);
}

