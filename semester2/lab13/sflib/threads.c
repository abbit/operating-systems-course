#include "threads.h"

static int sflib_error_code;

pthread_mutex_t *sf_mtx_create(pthread_mutexattr_t *attr) {
  pthread_mutex_t *mtx = malloc(sizeof(pthread_mutex_t));
  if ((sflib_error_code = pthread_mutex_init(mtx, attr))) {
    fprintf(stderr, "pthread_mutex_init error: (%d)%s\n", sflib_error_code,
            strerror(sflib_error_code));
    exit(EXIT_FAILURE);
  }
  return mtx;
}

void sf_mtx_destroy(pthread_mutex_t **mtx) {
  if ((sflib_error_code = pthread_mutex_destroy(*mtx))) {
    fprintf(stderr, "pthread_mutex_destroy error: (%d)%s\n", sflib_error_code,
            strerror(sflib_error_code));
    exit(EXIT_FAILURE);
  }
  free(*mtx);
  *mtx = NULL;
}

void sf_mtx_lock(pthread_mutex_t *mtx) {
  if ((sflib_error_code = pthread_mutex_lock(mtx))) {
    fprintf(stderr, "pthread_mutex_lock error: (%d)%s\n", sflib_error_code,
            strerror(sflib_error_code));
    exit(EXIT_FAILURE);
  }
}

void sf_mtx_unlock(pthread_mutex_t *mtx) {
  if ((sflib_error_code = pthread_mutex_unlock(mtx))) {
    fprintf(stderr, "pthread_mutex_unlock error: (%d)%s\n", sflib_error_code,
            strerror(sflib_error_code));
    exit(EXIT_FAILURE);
  }
}

pthread_cond_t *sf_cond_create(pthread_condattr_t *attr) {
  pthread_cond_t *cond = malloc(sizeof(pthread_cond_t));
  if ((sflib_error_code = pthread_cond_init(cond, attr))) {
    fprintf(stderr, "pthread_cond_init error: (%d)%s\n", sflib_error_code,
            strerror(sflib_error_code));
    exit(EXIT_FAILURE);
  }
  return cond;
}

void sf_cond_destroy(pthread_cond_t **cond) {
  if ((sflib_error_code = pthread_cond_destroy(*cond))) {
    fprintf(stderr, "pthread_cond_destroy error: (%d)%s\n", sflib_error_code,
            strerror(sflib_error_code));
    exit(EXIT_FAILURE);
  }
  free(*cond);
  *cond = NULL;
}

void sf_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mtx) {
  if ((sflib_error_code = pthread_cond_wait(cond, mtx))) {
    fprintf(stderr, "pthread_cond_wait error: (%d)%s\n", sflib_error_code,
            strerror(sflib_error_code));
    exit(EXIT_FAILURE);
  }
}

void sf_cond_signal(pthread_cond_t *cond) {
  if ((sflib_error_code = pthread_cond_signal(cond))) {
    fprintf(stderr, "pthread_cond_signal error: (%d)%s\n", sflib_error_code,
            strerror(sflib_error_code));
    exit(EXIT_FAILURE);
  }
}

void sf_cond_broadcast(pthread_cond_t *cond) {
  if ((sflib_error_code = pthread_cond_broadcast(cond))) {
    fprintf(stderr, "pthread_cond_broadcast error: (%d)%s\n", sflib_error_code,
            strerror(sflib_error_code));
    exit(EXIT_FAILURE);
  }
}

pthread_t sf_pth_create(const pthread_attr_t *restrict attr,
                        void *(*thread_func)(void *), void *restrict arg) {
  pthread_t tid;
  if ((sflib_error_code = pthread_create(&tid, attr, thread_func, arg))) {
    fprintf(stderr, "pthread_create error: (%d)%s\n", sflib_error_code,
            strerror(sflib_error_code));
    exit(EXIT_FAILURE);
  }

  return tid;
}

void sf_pth_join(pthread_t thread, void **status) {
  if ((sflib_error_code = pthread_join(thread, status))) {
    fprintf(stderr, "pthread_join error: (%d)%s\n", sflib_error_code,
            strerror(sflib_error_code));
    exit(EXIT_FAILURE);
  }
}
