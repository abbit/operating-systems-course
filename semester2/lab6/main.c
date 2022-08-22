#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define TIMES 100
#define BUFFER_SIZE 1024
#define SLEEP_DURATION_MULTIPLIER 100

void *thread_func(void *param) {
    char *str = (char *)param;
    int num = strlen(str);
    usleep(SLEEP_DURATION_MULTIPLIER * num);
    printf("%s\n", str);
    pthread_exit(0);
}

int main(int argc, char **argv) {
    pthread_t tid[TIMES];
    int err;

    char *arr_of_strings[TIMES] = {0};
    for (int i = 0; i < TIMES; i++) {
        arr_of_strings[i] = malloc(BUFFER_SIZE * sizeof(char));
        if(scanf("%s", arr_of_strings[i]) <= 0) {
           break;
       }
    }

    for (int i = 0; arr_of_strings[i] != 0; i++) {
        if ((err = pthread_create(&tid[i], NULL, thread_func, arr_of_strings[i]))) {
            fprintf(stderr, "pthread_create error: (%d)%s\n", err, strerror(err));
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; arr_of_strings[i] != 0; i++) {
        if ((err = pthread_join(tid[i], NULL))) {
            fprintf(stderr, "pthread_cancel error: (%d)%s\n", err, strerror(err));
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}
