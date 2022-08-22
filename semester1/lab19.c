#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool isMatchPattern(char *pattern, char *name) {
  int res = fnmatch(pattern, name, FNM_PERIOD);

  if (res == 0) return true;

  return false;
}

int main(int argc, char **argv) {
  DIR *dir = opendir(".");
  struct dirent *next;

  if (argc < 2) {
    perror("No pattern was provided\n");
    return 1;
  }

  while (true) {
    /* To distinguish end of stream from an error, set errno to zero before
       calling readdir() and then check the value of errno if NULL is returned.
     */
    errno = 0;
    next = readdir(dir);

    if (next) {
      if (isMatchPattern(argv[1], next->d_name)) {
        printf("%s\n", next->d_name);
      }
    } else if (errno != 0) {
      perror("Error while reading current directory\n");
      return 1;
    } else
      break;
  }

  return 0;
}