#include "sflib/sflib.h"

pthread_mutex_t *wait_first_mtx;
pthread_mutex_t *wait_second_mtx;
pthread_mutex_t *wait_between_mtx;

void *thread_func(void *_) {
  sf_mtx_lock(wait_second_mtx);
  for (int i = 0; i < 10; i++) {
    sf_mtx_lock(wait_first_mtx);
    printf("line #%d from created thread\n", i + 1);
    sf_mtx_unlock(wait_second_mtx);
    sf_mtx_lock(wait_between_mtx);
    sf_mtx_unlock(wait_first_mtx);
    sf_mtx_lock(wait_second_mtx);
    sf_mtx_unlock(wait_between_mtx);
  }

  pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  pthread_mutexattr_t *mtx_attr = malloc(sizeof(pthread_mutexattr_t));
  pthread_mutexattr_init(mtx_attr);
  pthread_mutexattr_settype(mtx_attr, PTHREAD_MUTEX_ERRORCHECK);

  wait_first_mtx = malloc(sizeof(pthread_mutex_t));
  wait_second_mtx = malloc(sizeof(pthread_mutex_t));
  wait_between_mtx = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(wait_first_mtx, mtx_attr);
  pthread_mutex_init(wait_second_mtx, mtx_attr);
  pthread_mutex_init(wait_between_mtx, mtx_attr);

  sf_mtx_lock(wait_first_mtx);

  pthread_t tid = sf_pth_create(NULL, thread_func, NULL);

  for (int i = 0; i < 10; i++) {
    printf("line #%d from main thread\n", i + 1);
    sf_mtx_lock(wait_between_mtx);
    sf_mtx_unlock(wait_first_mtx);
    sf_mtx_lock(wait_second_mtx);
    sf_mtx_unlock(wait_between_mtx);
    sf_mtx_lock(wait_first_mtx);
    sf_mtx_unlock(wait_second_mtx);
  }

  sf_pth_join(tid, NULL);
  pthread_mutex_destroy(wait_first_mtx);
  pthread_mutex_destroy(wait_second_mtx);
  pthread_mutex_destroy(wait_between_mtx);
  free(wait_first_mtx);
  free(wait_second_mtx);
  free(wait_second_mtx);

  return EXIT_SUCCESS;
}
