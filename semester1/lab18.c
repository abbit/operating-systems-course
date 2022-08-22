#include <errno.h>
#include <grp.h>
#include <libgen.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

void printFileType(struct stat stat) {
  if (S_ISREG(stat.st_mode)) {
    printf("-");
  } else if (S_ISDIR(stat.st_mode)) {
    printf("d");
  } else {
    printf("?");
  }
}

int main(int argc, char **argv) {
  struct stat fileStat;
  for (int i = 1; i < argc; ++i) {
    lstat(argv[i], &fileStat);
    if (errno) {
      perror("Error getting file stat");
    }

    printFileType(fileStat);

    printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");

    printf("\t%u\t", fileStat.st_nlink);

    struct passwd *pwuid = getpwuid(fileStat.st_uid);
    if (errno) {
      perror("Error getting user info");
      exit(-1);
    }
    printf("%s\t", pwuid->pw_name);

    struct group *grgid = getgrgid(fileStat.st_gid);
    if (errno) {
      perror("Error getting group info");
      exit(-1);
    }
    printf("%s\t", grgid->gr_name);

    if (S_ISREG(fileStat.st_mode)) {
      printf("%ld\t", fileStat.st_size);
    } else {
      printf("\t");
    }

    char *time = ctime(&fileStat.st_mtim.tv_sec);
    if (errno) {
      perror("Error ctime");
      exit(-1);
    }

    time[24] = 0;
    printf("%s\t", time);
    if (argv[i][(strlen(argv[i]) - 1)] == '/') {
      argv[i][(strlen(argv[i]) - 1)] = 0;
    }
    printf("%s\n", basename(argv[i]));
  }
  return 0;
}