#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

char *SOCKET_PATH = "socket";

void err_with_close(char *message, int fd) {
  perror(message);
  close(fd);
  unlink(SOCKET_PATH);
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

  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    err_with_close("error on binding socket", sock);
  }

  if (listen(sock, 5) < 0) {
    err_with_close("error on listening socket", sock);
  }

  int connection = accept(sock, NULL, NULL);
  if (connection < 0) {
    err_with_close("error on accepting connection", sock);
  }

  char buffer[BUFSIZ];
  int message_size;
  while ((message_size = read(connection, buffer, sizeof(buffer))) > 0) {
    for (int i = 0; i < message_size; ++i) {
      buffer[i] = toupper(buffer[i]);
    }
    write(STDOUT_FILENO, buffer, message_size);
  }

  if (message_size < 0) {
    err_with_close("error on reading from socket", sock);
  }

  if ((close(connection)) < 0) {
    err_with_close("error on closing connection", sock);
  }

  if ((close(sock)) < 0) {
    err_with_close("error on closing socket", sock);
  }

  if ((unlink(SOCKET_PATH)) < 0) {
    err_with_close("error on unlinking socket", sock);
  }

  return 0;
}