#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

const char *SOCKET_PATH = "socket";

void err_with_close(char *message, int fd) {
  perror(message);
  close(fd);
  exit(-1);
}

int main(int argc, char **argv) {
  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

  int sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("error on creating socket");
    exit(-1);
  }

  if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    err_with_close("error on connecting to socket", sock);
  }

  char buffer[BUFSIZ];
  int message_size;
  while ((message_size = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
    int written_size = write(sock, buffer, message_size);
    if (written_size < 0) {
      err_with_close("error on writing to socket", sock);
    }

    if (written_size != message_size) {
      fprintf(stderr, "partial write\n");
    }
  }

  if (message_size < 0) {
    err_with_close("error on reading", sock);
  }

  if ((close(sock)) < 0) {
    perror("error on closing socket");
    exit(-1);
  }

  return 0;
}