// socket.c
// frfom here http://etherealwake.com/2021/01/portable-sockets-basics/
#include "socket.h"
#include <stdio.h>
#include <string.h>

int socket_startup() {
#ifdef _WIN32
  WSADATA wsadata;
  int error = WSAStartup(MAKEWORD(2, 2), &wsadata);
  if (error) return error;

  if (wsadata.wVersion < MAKEWORD(2, 2)) {
    WSACleanup();
    return WSAVERNOTSUPPORTED;
  }
#endif

  return 0;
}

int socket_strerror(int error, char *buffer, size_t buflen) {
#if _WIN32
  DWORD length = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
                                error, 0, buffer, buflen, NULL);
  if (length == 0) return -1;
  char *eol = strchr(buffer, '\n');
  if (eol != NULL) *eol = '\0';
  return 0;
#else
  // Can be replaced with strlcpy if known to be available
  snprintf(buffer, buflen, "%s", strerror(error));
  return 0;
#endif
}

// Instantiate Inline Functions.
int inprogress(int error);
int last_socket_error(void);
int get_cloexec(socket_t sock);
int set_cloexec(socket_t sock, int value);
int socket_cleanup(void);
socket_t socket4(int domain, int type, int protocol, int flags);
#ifndef _WIN32
int closesocket(int sock);
int get_nonblock(socket_t sock);
int set_nonblock(socket_t sock, int value);
#else
int poll(struct pollfd *fds, int nfds, int timeout);
int set_nonblock(socket_t sock, u_long value);
ssize_t recvmsg(socket_t sock, struct msghdr *msg, DWORD flags);
ssize_t sendmsg(socket_t sock, const struct msghdr *msg, DWORD flags);
#endif