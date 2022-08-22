#include "sflib/threads.h"

pthread_mutex_t *mtx;
pthread_cond_t *cond;

void *thread_func(void *_) {
  sf_mtx_lock(mtx);
  sf_cond_signal(cond);
  for (int i = 0; i < 10; i++) {
    sf_cond_wait(cond, mtx);
    printf("line #%d from created thread\n", i + 1);
    sf_cond_signal(cond);
  }
  sf_mtx_unlock(mtx);

  pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  mtx = sf_mtx_create(NULL);
  cond = sf_cond_create(NULL);

  sf_mtx_lock(mtx);
  pthread_t tid = sf_pth_create(NULL, thread_func, NULL);
  sf_cond_wait(cond, mtx);
  for (int i = 0; i < 10; i++) {
    printf("line #%d from main thread\n", i + 1);
    sf_cond_signal(cond);
    sf_cond_wait(cond, mtx);
  }
  sf_mtx_unlock(mtx);

  sf_pth_join(tid, NULL);
  sf_mtx_destroy(mtx);
  sf_cond_destroy(&cond);

  return EXIT_SUCCESS;
}
