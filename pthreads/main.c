#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define N 5

static void *run(void *arg) {
    size_t job = *(size_t*)arg;
    printf("Job %zu\n", job);
    return NULL;
}

int main(int argc, char *argv[]) {
   size_t jobs[N];
   pthread_t threads[N];
   for (size_t i=0; i<N; ++i) {
       jobs[i] = i;
       pthread_create(threads+i, NULL, run, jobs+i);
   }

   for (size_t i=0; i<N; ++i) {
       pthread_join(threads[i], NULL);
   }

   return EXIT_SUCCESS;
}