#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>

// 4 Mb
#define DEFAULT_BODY_SIZE 4194304
// 1 Mb
#define RESPONSE_HEADERS_MAX_SIZE 1048576
// 1 Kb
#define RESPONSE_BUFFER_SIZE 1024
// 1 Kb
#define REQUEST_MAX_SIZE 1024
#define HOST_MAX_LENGTH 256
#define PATH_MAX_LENGTH 2084
#define PRINTED_LINES_COUNT 25
// 10 sec
#define TIMEOUT 10000
#define POLLFD_SIZE 2

void *sfmalloc(size_t size) {
  errno = 0;
  void *arr = NULL;
  arr = malloc(size);
  if (errno || arr == NULL) {
    perror("error allocating memory");
    exit(EXIT_FAILURE);
  }
  return arr;
}

void *sfcalloc(size_t count, size_t size) {
  void *arr = sfmalloc(size * count);
  memset(arr, 0, size * count);
  return arr;
}

void error_exit_with_message(char *message, int fd) {
  perror(message);
  close(fd);
  exit(EXIT_FAILURE);
}

void refresh_revents(struct pollfd *pollfds) {
  for (int i = 0; i < POLLFD_SIZE; i++) {
    pollfds[i].revents = 0;
  }
}

void init_pollfd(struct pollfd *pollfds, int sock_fd) {
  pollfds[0].fd = sock_fd;
  pollfds[1].fd = STDIN_FILENO;
  for (int i = 0; i < POLLFD_SIZE; i++) {
    pollfds[i].events = POLLIN;
  }

  refresh_revents(pollfds);
}

typedef struct {
  char *headers;
  size_t headers_size;

  char *body;
  size_t body_size;
  size_t body_first_empty_index;

  int is_header_skipped;
  int is_conn_closed;
} Response;

Response *Response_new() {
  Response *resp = (Response *)sfmalloc(sizeof(Response));
  resp->headers = (char *)sfcalloc(sizeof(char), RESPONSE_HEADERS_MAX_SIZE);
  resp->headers_size = 0;

  resp->body = NULL;
  resp->body_size = 0;
  resp->body_first_empty_index = 0;

  resp->is_header_skipped = 0;
  resp->is_conn_closed = 0;

  return resp;
}

void Response_free(Response **resp) {
  free((*resp)->headers);
  free((*resp)->body);
  free(*resp);
  *resp = NULL;
}

typedef struct {
  char *host;
  char *path;
} Url;

Url *Url_new() {
  Url *url = (Url *)sfmalloc(sizeof(Url));
  url->host = (char *)sfcalloc(sizeof(char), HOST_MAX_LENGTH);
  url->path = (char *)sfcalloc(sizeof(char), PATH_MAX_LENGTH);

  return url;
}

void Url_free(Url **url) {
  free((*url)->host);
  free((*url)->path);
  free(*url);
  *url = NULL;
}

Url *parse_url(char *url) {
  Url *parsed = Url_new();

  sscanf(url, "http://%99[^/]%99[^\n]", parsed->host, parsed->path);

  if (parsed->path[0] == '\0') {
    parsed->path[0] = '/';
    parsed->path[1] = '\0';
  }

  return parsed;
}

int connect_to_host(char *hostname) {
  struct addrinfo hints;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  struct addrinfo *servinfo;
  if (getaddrinfo(hostname, "http", &hints, &servinfo) != 0) {
    fprintf(stderr, "error on getaddrinfo\n");
    exit(EXIT_FAILURE);
  }

  int sock_fd = -1;
  for (struct addrinfo *p = servinfo; p != NULL; p = p->ai_next) {
    sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sock_fd == -1) continue;

    if (connect(sock_fd, p->ai_addr, p->ai_addrlen) == 0) {
      struct sockaddr_in *in = (struct sockaddr_in *)p->ai_addr;
      printf("Connected to %s (%s), port %d\n", hostname,
             inet_ntoa(in->sin_addr), ntohs(in->sin_port));
      return sock_fd;
    }

    close(sock_fd);
  }

  return sock_fd;
}

char *find_start_of_body(char *resp, size_t length) {
  char *mark = "\r\n\r\n";
  size_t mark_curr_pos = 0;
  size_t mark_len = 4;

  for (size_t i = 0; i < length; i++) {
    if (mark_curr_pos == mark_len) return resp + i;

    if (mark[mark_curr_pos] == resp[i]) {
      mark_curr_pos++;
    } else {
      mark_curr_pos = 0;
    }
  }

  return NULL;
}

size_t get_body_size(Response *resp) {
  size_t body_size = DEFAULT_BODY_SIZE;

  char *len_start;
  if ((len_start = strcasestr(resp->headers, "content-length"))) {
    len_start += 16;  // skip "Content-Length: "

    body_size = strtoul(len_start, NULL, 10);
  }

  return body_size;
}

void read_data(int sock_fd, Response *resp) {
  char buffer[RESPONSE_BUFFER_SIZE];
  int response_message_size = read(sock_fd, buffer, RESPONSE_BUFFER_SIZE);

  if (resp->is_conn_closed && response_message_size == 0) {
    resp->body_size = resp->body_first_empty_index;
    return;
  }

  if (resp->is_header_skipped) {
    strncpy(resp->body + resp->body_first_empty_index, buffer,
            response_message_size);
    resp->body_first_empty_index += response_message_size;

    return;
  }

  char *start_of_body = find_start_of_body(buffer, response_message_size);

  if (start_of_body) {
    size_t header_size = start_of_body - buffer;
    strncpy(resp->headers + resp->headers_size, buffer, header_size);
    resp->headers_size += header_size;

    resp->body_size = get_body_size(resp);
    free(resp->body);
    resp->body = (char *)sfcalloc(sizeof(char), resp->body_size + 10);

    size_t body_size = response_message_size - header_size;
    strncpy(resp->body + resp->body_first_empty_index, buffer + header_size,
            body_size);
    resp->body_first_empty_index += body_size;

    resp->is_header_skipped = 1;

    return;
  }

  strncpy(resp->headers + resp->headers_size, buffer, response_message_size);
  resp->headers_size += response_message_size;
}

void read_input(int *max_available_lines) {
  int c = getchar();
  if (c == ' ') {
    *max_available_lines += PRINTED_LINES_COUNT;
  }
}

char *create_request(Url *url) {
  char *request = (char *)sfcalloc(sizeof(char), REQUEST_MAX_SIZE);
  sprintf(request, "GET %s HTTP/1.0\r\n\r\n", url->path);
  printf("Request:\n%s\n", request);
  return request;
}

void make_request(int sock_fd, Url *url) {
  char *request = create_request(url);
  int written_size = write(sock_fd, request, strlen(request));
  if (written_size < 0) {
    free(request);
    shutdown(sock_fd, SHUT_WR);
    error_exit_with_message("error on writing to socket", sock_fd);
  }

  if (written_size != strlen(request)) {
    fprintf(stderr, "partial write\n");
  }

  free(request);
  shutdown(sock_fd, SHUT_WR);
}

void print_data(Response *resp, int *body_idx, int *printed_lines,
                int max_available_lines, int *scrolled_to_end) {
  if (resp->body && *printed_lines < max_available_lines) {
    for (; *printed_lines < max_available_lines; (*printed_lines)++) {
      // print line
      for (;;) {
        if (*body_idx >= resp->body_size) {
          *scrolled_to_end = 1;
          return;
        }

        if (*body_idx >= resp->body_first_empty_index) {
          return;
        }

        if (resp->body[*body_idx] == '\n') {
          // print '\n'
          putchar(resp->body[*body_idx]);
          (*body_idx)++;
          break;
        }

        putchar(resp->body[*body_idx]);
        (*body_idx)++;
      }
    }

    printf("Press space to scroll down\n");
  }
}

int main(int argc, char **argv) {
  // Эти строки нужны, чтобы нормально считывалось нажатие пробела
  static struct termios t;
  if (tcgetattr(STDIN_FILENO, &t) == -1) {
    perror("tcgetattr error: ");
    exit(EXIT_FAILURE);
  }
  t.c_lflag &= ~(ICANON);
  if (tcsetattr(STDIN_FILENO, TCSANOW, &t) == -1) {
    perror("tcsetattr error: ");
    exit(EXIT_FAILURE);
  }

  if (argc != 2) {
    fprintf(stderr, "Usage: %s URL\n", argv[0]);
    return -1;
  }

  Url *url = parse_url(argv[1]);

  int sock_fd = connect_to_host(url->host);
  if (sock_fd < 0) {
    Url_free(&url);
    perror("error on creating socket");
    exit(EXIT_FAILURE);
  }

  make_request(sock_fd, url);
  Url_free(&url);

  struct pollfd pollfds[POLLFD_SIZE];
  init_pollfd(pollfds, sock_fd);

  Response *response = Response_new();

  int scrolled_to_end = 0;
  int max_available_lines = PRINTED_LINES_COUNT;
  int printed_lines = 0;
  int body_idx = 0;

  while (!scrolled_to_end) {
    int poll_result = poll(pollfds, 2, TIMEOUT);

    if (poll_result < 0) {
      error_exit_with_message("error on poll", sock_fd);
    }

    if (poll_result == 0) {
      break;
    }

    if (pollfds[0].revents & POLLHUP) {
      response->is_conn_closed = 1;
    }

    if (pollfds[0].revents & POLLIN) {
      read_data(sock_fd, response);
    }

    if (pollfds[1].revents & POLLIN) {
      read_input(&max_available_lines);
    }

    print_data(response, &body_idx, &printed_lines, max_available_lines,
               &scrolled_to_end);

    refresh_revents(pollfds);
  }

  Response_free(&response);
  if ((close(sock_fd)) < 0) {
    perror("error on closing socket");
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}
