#include "sflib/sflib.h"

void *thread_func(void *param) {
  int num = *(int *)param;
  printf("Hello, %d!\n", num);

  pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
  pthread_attr_t tattr;
  int err;

  pthread_attr_init(&tattr);

  int a = 5;
  pthread_t tid = sf_pth_create(&tattr, thread_func, &a);
  sf_pth_join(tid, NULL);

  return EXIT_SUCCESS;
}
