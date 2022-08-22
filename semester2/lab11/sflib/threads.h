#ifndef SF_H
#define SF_H

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define min(a, b)       \
  ({                    \
    typeof(a) _a = (a); \
    typeof(b) _b = (b); \
    _a < _b ? _a : _b;  \
  })

#define max(a, b)       \
  ({                    \
    typeof(a) _a = (a); \
    typeof(b) _b = (b); \
    _a > _b ? _a : _b;  \
  })

pthread_mutex_t *sf_mtx_create(pthread_mutexattr_t *attr);

void sf_mtx_destroy(pthread_mutex_t **mtx);

void sf_mtx_lock(pthread_mutex_t *mtx);

void sf_mtx_unlock(pthread_mutex_t *mtx);

pthread_cond_t *sf_cond_create(pthread_condattr_t *attr);

void sf_cond_destroy(pthread_cond_t **cond);

void sf_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mtx);

void sf_cond_signal(pthread_cond_t *cond);

void sf_cond_broadcast(pthread_cond_t *cond);

pthread_t sf_pth_create(const pthread_attr_t *restrict attr,
                        void *(*thread_func)(void *), void *restrict arg);

void sf_pth_join(pthread_t thread, void **status);

#endif
