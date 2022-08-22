#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PHILOSOPHERS_COUNT 5
#define DELAY 30000
#define FOOD 50

void *philosopher(void *id);
int food_on_table();
void get_fork(int, int, int);
void down_forks(int, int);

pthread_mutex_t forks[PHILOSOPHERS_COUNT];
pthread_t phils[PHILOSOPHERS_COUNT];
pthread_mutex_t foodlock;

void *philosopher(void *num) {
  int id = (int)num;
  printf("Philosopher %d sitting down to dinner.\n", id);

  int right_fork = id;
  int left_fork = id + 1;
  if (left_fork == PHILOSOPHERS_COUNT) left_fork = 0;

  int high_prior_fork = max(left_fork, right_fork);
  int low_prior_fork = min(left_fork, right_fork);
  int is_high_left = high_prior_fork == left_fork;

  int f;
  while (f = food_on_table()) {
    printf("Philosopher %d: get dish %d.\n", id, f);
    get_fork(id, high_prior_fork, is_high_left);
    get_fork(id, low_prior_fork, !is_high_left);

    printf("Philosopher %d: eating.\n", id);
    usleep(DELAY * (FOOD - f + 1));
    down_forks(left_fork, right_fork);
  }
  printf("Philosopher %d is done eating.\n", id);

  return NULL;
}

int food_on_table() {
  static int food = FOOD;
  int myfood;

  pthread_mutex_lock(&foodlock);
  if (food > 0) {
    food--;
  }
  myfood = food;
  pthread_mutex_unlock(&foodlock);

  return myfood;
}

void get_fork(int phil, int fork, int is_left) {
  pthread_mutex_lock(&forks[fork]);
  printf("Philosopher %d: got %s fork %d\n", phil, is_left ? "left" : "right",
         fork);
}

void down_forks(int f1, int f2) {
  pthread_mutex_unlock(&forks[f1]);
  pthread_mutex_unlock(&forks[f2]);
}

int main(int argn, char **argv) {
  pthread_mutex_init(&foodlock, NULL);

  for (int i = 0; i < PHILOSOPHERS_COUNT; i++) {
    pthread_mutex_init(&forks[i], NULL);
  }

  for (int i = 0; i < PHILOSOPHERS_COUNT; i++) {
    pthread_create(&phils[i], NULL, philosopher, (void *)i);
  }

  for (int i = 0; i < PHILOSOPHERS_COUNT; i++) {
    pthread_join(phils[i], NULL);
  }

  for (int i = 0; i < PHILOSOPHERS_COUNT; i++) {
    pthread_mutex_destroy(&forks[i]);
  }

  return 0;
}
