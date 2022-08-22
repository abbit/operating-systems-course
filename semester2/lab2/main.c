#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

void *thread_func(void *_) {
    for (int i=0; i<10; i++) {
        printf("line #%d from created thread\n", i+1);
    }

    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    pthread_t tid;

    int err = pthread_create(&tid, NULL, thread_func, NULL);
    if(err) {
        perror("Error while creating pthread");
    }

    pthread_join(tid, NULL);

    for (int i=0; i<10; i++) {
        printf("line #%d from main thread\n", i+1);
    }

    return EXIT_SUCCESS;
}
