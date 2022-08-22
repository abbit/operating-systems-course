#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define SLEEP_DURATION 2

void *thread_func(void *param) {
    while(1)
        printf("Hello, there from created pthread!\n");
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

    return EXIT_SUCCESS;
}
