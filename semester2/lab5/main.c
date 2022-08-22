#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define SLEEP_DURATION 2

void thread_preexit(void *param) {
    printf("Thread going to finish\n");
}

void *thread_func(void *param) {
    pthread_cleanup_push(thread_preexit, NULL);
    while(1) {
        printf("line from created thread\n");
    }
    pthread_cleanup_pop(1);
}

int main(int argc, char **argv) {
    pthread_t tid;
    pthread_attr_t tattr;
    int err;

    pthread_attr_init(&tattr);

    if ((err = pthread_create(&tid, &tattr, thread_func, NULL))) {
        fprintf(stderr, "pthread_create error: (%d)%s\n", err, strerror(err));
        exit(EXIT_FAILURE);
    }

    sleep(SLEEP_DURATION);

    if ((err = pthread_cancel(tid))) {
        fprintf(stderr, "pthread_cancel error: (%d)%s\n", err, strerror(err));
        exit(EXIT_FAILURE);
    }
    sleep(1);
    return EXIT_SUCCESS;
}
