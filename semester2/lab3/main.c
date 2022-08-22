#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define NUM_OF_THREADS 4
#define STRINGS_COUNT 3
#define MAX_STRING_LEN 100

void *thread_func(void *param) {
    char **strings = (char **)param;
    
    for (int i=0; i < STRINGS_COUNT; i++) {
        printf("%s\n", strings[i]);
    }
    
    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    pthread_t tid[NUM_OF_THREADS];

    char ***strings_for_threads = calloc(NUM_OF_THREADS, sizeof(char**));
    for (int i=0; i < NUM_OF_THREADS; i++) {
        strings_for_threads[i] = calloc(STRINGS_COUNT, sizeof(char*));
        for (int j=0; j < STRINGS_COUNT; j++) {
            strings_for_threads[i][j] = calloc(MAX_STRING_LEN, sizeof(char));
            snprintf(strings_for_threads[i][j], MAX_STRING_LEN, "thread %d, string %d", i+1, j+1);
        }
    }

    for (int i=0; i < NUM_OF_THREADS; i++) {
        int err = pthread_create(&tid[i], NULL, thread_func, strings_for_threads[i]);
        if (err) {
            fprintf(stderr, "pthread_create error: (%d)%s\n", err, strerror(err));
            exit(EXIT_FAILURE);
        }
    }

    for (int i=0; i < NUM_OF_THREADS; i++) {
        int err = pthread_join(tid[i], NULL);
        if (err) {
            fprintf(stderr, "pthread_join error: (%d)%s\n", err, strerror(err));
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}
