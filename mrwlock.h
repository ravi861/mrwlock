typedef struct mrwlock_s {
    unsigned int __writer;
} mrwlock_t;

extern int mrwlock_init (mrwlock_t *mrwlock);
extern int mrwlock_rdlock (mrwlock_t *mrwlock);
extern int mrwlock_wrlock (mrwlock_t *mrwlock);
extern int mrwlock_unlock (mrwlock_t *mrwlock);
