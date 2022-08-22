#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <ulimit.h>
#include <unistd.h>

extern char **environ;

int main(int argc, char *argv[]) {
  char options[] = "ispuU:cC:dvV:", *tmp;
  struct rlimit r;
  long newValue;
  int c;

  while ((c = getopt(argc, argv, options)) != EOF) {
    switch (c) {
      case 'i':
        printf("real user ID: %d\n", getuid());
        printf("effective user ID: %d\n", geteuid());
        printf("real group ID: %d\n", getgid());
        printf("effective group ID: %d\n", getegid());
        break;
      case 's':
        setpgrp();
        break;
      case 'p':
        printf("process ID: %d\n", getpid());
        printf("process' parent ID: %d\n", getppid());
        printf("real group ID: %d\n", getgid());
        break;
      case 'u':
        printf("ulimit: %ld\n",
               ulimit(UL_GETFSIZE, 0));  // in units of 512 bytes
        break;
      case 'U':
        errno = 0;
        newValue = strtol(optarg, &tmp, 10);
        if (*tmp != '\0' || newValue < 0) {
          errno = EINVAL;
        } else {
          ulimit(UL_SETFSIZE, newValue);
        }
        if (errno) {
          perror("ERROR");
        }
        break;
      case 'c':
        getrlimit(RLIMIT_CORE, &r);
        printf("size of a core file: %lu\n", r.rlim_cur);
        break;
      case 'C':
        errno = 0;
        newValue = strtol(optarg, &tmp, 10);
        if (*tmp != '\0' || newValue < 0) {
          errno = EINVAL;
        } else {
          getrlimit(RLIMIT_CORE, &r);
          r.rlim_cur = newValue;
          setrlimit(RLIMIT_CORE, &r);
        }
        if (errno) {
          perror("ERROR");
        }
        break;
      case 'd':
        tmp = getcwd(NULL, PATH_MAX);
        if (tmp == NULL) {
          perror("ERROR");
        } else {
          printf("%s\n", tmp);
          free(tmp);
        }
        break;
      case 'v':
        printf("environment variables:\n");
        for (char **p = environ; *p; p++) {
          printf("%s\n", *p);
        }
        break;
      case 'V':
        if (putenv(optarg)) {
          perror("ERROR");
        }
        break;
    }
  }
  return 0;
}