#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const unsigned long long STEPS = 100000000;

long double calcPiPart(unsigned long long offset, unsigned long long steps) {
  long double pi = 0.0;

  unsigned long long end = offset + steps;
  for (unsigned long long i = offset; i < end; i++) {
    pi += 1.0 / (i * 4.0 + 1.0);
    pi -= 1.0 / (i * 4.0 + 3.0);
  }

  return pi;
}

typedef struct task {
  unsigned long long offset;
  unsigned long long steps;
  struct task *next;
} Task;

Task *Task_new(unsigned long long offset) {
  Task *new = calloc(1, sizeof(Task));
  new->offset = offset;
  new->steps = STEPS;
  new->next = NULL;

  return new;
}

typedef struct {
  Task *first;
  Task *last;
  pthread_mutex_t *mtx_queue;
  bool is_running;
  long double res;
  size_t workers_count;
  pthread_t *worker_ids;
} ThreadPool;

void ThreadPool_pushTask(ThreadPool *th_pool) {
  if (th_pool == NULL) return;

  Task *last = th_pool->last;
  if (last == NULL) return;

  unsigned long long new_offset = last->offset + last->steps;

  if (new_offset < last->offset) return;

  last->next = Task_new(new_offset);
  th_pool->last = last->next;
}

Task *ThreadPool_popTask(ThreadPool *th_pool) {
  if (th_pool == NULL) return NULL;

  Task *task = th_pool->first;
  if (task == NULL) return NULL;

  th_pool->first = task->next;

  return task;
}

void *worker(void *arg) {
  ThreadPool *th_pool = (ThreadPool *)arg;
  Task *task = NULL;

  long double *part_res = calloc(1, sizeof(long double));

  while (th_pool->is_running) {
    pthread_mutex_lock(th_pool->mtx_queue);
    task = ThreadPool_popTask(th_pool);
    ThreadPool_pushTask(th_pool);
    pthread_mutex_unlock(th_pool->mtx_queue);

    if (task == NULL) break;

    printf("Start calculations with offset: %llu\n", task->offset);

    *part_res += calcPiPart(task->offset, task->steps);
    free(task);
    task = NULL;
  }

  pthread_exit(part_res);
}

ThreadPool *ThreadPool_new(size_t workers_count) {
  ThreadPool *new = calloc(1, sizeof(ThreadPool));

  new->is_running = false;
  new->workers_count = workers_count;
  new->res = 0.0;

  new->worker_ids = calloc(new->workers_count, sizeof(pthread_t));

  new->mtx_queue = calloc(1, sizeof(pthread_mutex_t));
  pthread_mutex_init(new->mtx_queue, NULL);

  // init queue
  new->first = Task_new(0);
  new->last = new->first;
  for (size_t i = 1; i < new->workers_count; i++) {
    new->last->next = Task_new(new->last->offset + new->last->steps);
    new->last = new->last->next;
  }

  return new;
}

void ThreadPool_start(ThreadPool *th_pool) {
  if (th_pool == NULL) return;

  th_pool->is_running = true;

  int err;
  for (size_t i = 0; i < th_pool->workers_count; i++) {
    if ((err = pthread_create(&(th_pool->worker_ids[i]), NULL, worker,
                              th_pool))) {
      fprintf(stderr, "pthread_create error: (%d)%s\n", err, strerror(err));
      exit(EXIT_FAILURE);
    }
  }

  void *calculated_pi_part = NULL;
  for (size_t i = 0; i < th_pool->workers_count; i++) {
    if ((err = pthread_join(th_pool->worker_ids[i], &calculated_pi_part))) {
      fprintf(stderr, "pthread_join error: (%d)%s\n", err, strerror(err));
      exit(EXIT_FAILURE);
    }
    th_pool->res += *(long double *)calculated_pi_part;
    free(calculated_pi_part);
    calculated_pi_part = NULL;
  }
}

void ThreadPool_stop(ThreadPool *th_pool) {
  if (th_pool == NULL) return;

  th_pool->is_running = false;
}

void ThreadPool_destroy(ThreadPool *th_pool) {
  if (th_pool == NULL) return;

  if (th_pool->is_running) ThreadPool_stop(th_pool);

  // clean queue
  Task *tmp;
  while (th_pool->first != NULL) {
    tmp = th_pool->first->next;
    free(th_pool->first);
    th_pool->first = tmp;
  }

  // TODO: do unlock needed?
  pthread_mutex_unlock(th_pool->mtx_queue);
  pthread_mutex_destroy(th_pool->mtx_queue);
  free(th_pool->mtx_queue);

  free(th_pool);
}

ThreadPool *pool = NULL;

void sigIntHandler(int ignored) {
  printf("\nGot signal, will finish as soon as possible.\n");
  ThreadPool_stop(pool);
}

int main(int argc, char **argv) {
  int err;

  if (argc != 2) {
    fprintf(stderr,
            "Wrong number of arguments!\n"
            "Usage: ./compiled_lab9 [number of threads]\n");
    exit(EXIT_FAILURE);
  }

  int threads_num = 1;
  if ((threads_num = atoi(argv[1])) <= 0) {
    fprintf(stderr, "Number of threads must be positive!\n");
    exit(EXIT_FAILURE);
  }

  pool = ThreadPool_new(threads_num);
  signal(SIGINT, sigIntHandler);

  ThreadPool_start(pool);

  long double calculated_pi = pool->res;
  calculated_pi *= 4.0;
  printf("pi done - %.25Lg \n", calculated_pi);

  ThreadPool_destroy(pool);

  return EXIT_SUCCESS;
}
