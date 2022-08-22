#include <ctype.h>
#include <poll.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

const int MAX_CONNECTIONS = 2;
const int TIMEOUT_TIME = 10000;  // in ms
const char *SOCKET_PATH = "socket";

void close_pollfds(struct pollfd *pollfds) {
  for (int i = 0; i < MAX_CONNECTIONS + 1; i++) {
    if (pollfds->fd > 0) {
      close(pollfds[i].fd);
    }
  }
}

void err_with_close(char *message, int fd) {
  perror(message);
  close(fd);
  unlink(SOCKET_PATH);
  exit(-1);
}

void err_with_close_socks(char *message, struct pollfd *pollfds) {
  perror(message);
  close_pollfds(pollfds);
  unlink(SOCKET_PATH);
  exit(-1);
}

void refresh_revents(struct pollfd *pollfds) {
  for (int i = 0; i < MAX_CONNECTIONS + 1; i++) {
    pollfds[i].revents = 0;
  }
}

int accept_connection(int sock, struct pollfd *pollfds) {
  int conn_fd = accept(sock, NULL, NULL);
  if (conn_fd < 0) {
    err_with_close("error on accepting connection",
                   sock);  // TODO: mb ne exit?
  }

  int accepted = 0;
  for (int i = 0; i < MAX_CONNECTIONS; i++) {
    if (pollfds[i].fd == -1) {
      pollfds[i].fd = conn_fd;

      printf("Connection accepted\n");
      accepted = 1;
      break;
    }
  }

  if (!accepted) {
    printf("Connections limit exceeded!\n");
    close(conn_fd);
    // TODO: mb return?
  }

  return 0;
}

int close_connection(int idx, struct pollfd *pollfds) {
  if (pollfds[idx].fd > 0) {
    close(pollfds[idx].fd);
  }
  pollfds[idx].fd = -1;

  printf("Connection closed\n");

  return 0;
}

int read_message(int sock, int idx, struct pollfd *pollfds) {
  printf("received message from client #%d:\n", idx + 1);

  char buffer[BUFSIZ];
  int message_size = read(pollfds[idx].fd, buffer, sizeof(buffer));

  for (int i = 0; i < message_size; ++i) {
    buffer[i] = toupper(buffer[i]);
  }
  write(STDOUT_FILENO, buffer, message_size);

  if (message_size < 0) {
    err_with_close("error on reading from socket", sock);
  }

  return 0;
}

int init_socket() {
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

  return sock;
}

void close_server(struct pollfd *pollfds, int sock) {
  close_pollfds(pollfds);
  unlink(SOCKET_PATH);
}

int main(int argc, char **argv) {
  int sock = init_socket();

  struct pollfd pollfds[MAX_CONNECTIONS + 1];

  for (int i = 0; i < MAX_CONNECTIONS; i++) {
    pollfds[i].fd = -1;
    pollfds[i].events = POLLIN;
  }

  pollfds[MAX_CONNECTIONS].fd = sock;
  pollfds[MAX_CONNECTIONS].events = POLLIN;

  refresh_revents(pollfds);

  while (1) {
    int poll_result = poll(pollfds, MAX_CONNECTIONS + 1, TIMEOUT_TIME);

    if (poll_result < 0) {
      err_with_close_socks("error on poll", pollfds);
    }

    if (poll_result == 0) {
      break;
    }

    if (pollfds[MAX_CONNECTIONS].revents == POLLIN) {
      accept_connection(sock, pollfds);
    }

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
      if (pollfds[i].revents == POLLIN) {
        read_message(sock, i, pollfds);
      } else if (pollfds[i].revents == (POLLIN | POLLHUP)) {
        close_connection(i, pollfds);
      }
    }

    refresh_revents(pollfds);
  }

  printf("Closing server\n");

  close_server(pollfds, sock);

  return 0;
}