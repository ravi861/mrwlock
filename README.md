# Modified Read Write Lock mechanism

A modified read write lock which favors writes when the number of reads are significantly higher.

Build:
Use one of 3 below options, to run instrumentation code "thread_test"

1. make mutex
    Uses the pthread_mutex_t APIs
2. make rw
    Uses the phread_rwlock_t APIs
3. make mrw
    Uses the mrwlock_t APIs from libmrw.so
4. make libmrw
    Just build the library
    
I will upload the performance charts after I can clean them a bit but here is the table for 32 reader threads and 1 writer threads operating on 1MB of data. You will notice that for MRW lock, the write performance is almost an order of magnitude better than mutex and way better than RW locks.

            mutex        rw            mrw
Reads       9633081      4334686       9127972
Writes      282959       70247         1840049
