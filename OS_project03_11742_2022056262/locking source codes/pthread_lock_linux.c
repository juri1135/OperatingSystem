#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
int shared_resource = 0;

#define NUM_ITERS 500
#define NUM_THREADS 10000
int lk=0;
int owner=-1;
void lock(int *lk, int tid);
void unlock(int *lk, int tid);

static inline int xchg(volatile int *addr, int newval)
{
  int result;

  // The + in "+m" denotes a read-modify-write operand.
  asm volatile("lock; xchgl %0, %1" :
               "+m" (*addr), "=a" (result) :
               "1" (newval) :
               "cc");
  return result;
}

void lock(int *lk, int tid)
{   
    if(owner==tid){
        printf("panic: acquire. tid:%d\n",tid);
        exit(EXIT_FAILURE);
    }
    while(xchg(lk, 1) != 0)
        ;
    owner=tid;
}

void unlock(int *lk, int tid)
{   
    if (owner!= tid) {
        printf("panic: release lock from another thread tid:%d\n",tid);
        exit(EXIT_FAILURE);
    }
    if(owner==-1){
        printf("panic: release tid:%d\n",tid);
        exit(EXIT_FAILURE);
    }
    owner = -1;
    *lk=0;

}

void* thread_func(void* arg) {
    int tid = *(int*)arg;
    
    
    lock(&lk,tid);
        for(int i = 0; i < NUM_ITERS; i++)    shared_resource++;
    unlock(&lk,tid);
    pthread_exit(NULL);
}

int main() {
    int n=NUM_THREADS;
    pthread_t threads[n];
    int tids[n];
    
    for (int i = 0; i < n; i++) {
        tids[i] = i;
        pthread_create(&threads[i], NULL, thread_func, &tids[i]);
    }
    
    for (int i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("shared: %d\n", shared_resource);
    
    return 0;
}
