// socket.h
// from here: http://etherealwake.com/2021/01/portable-sockets-basics/
#ifndef SOCKET_H_
#define SOCKET_H_

#include <stdint.h>

// Declare Structures
struct iovec;
struct msghdr;

// Platform Specific Types
#ifndef _WIN32
typedef int socket_t;
#else
typedef uintptr_t socket_t;
// These types are missing from WinSock
typedef int socklen_t;
typedef intptr_t ssize_t;
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <WinSock2.h>
#include <Ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Constants
#ifndef _WIN32
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR   (-1)
#else
#define SOCK_CLOEXEC   WSA_FLAG_NO_HANDLE_INHERIT
#define SOCK_NONBLOCK  (+1)  // Arbitrary Value
#define SHUT_WR        SD_SEND
#define SHUT_RD        SD_RECV
#define SHUT_RDWR      SD_BOTH
#endif

// Types
#ifdef _WIN32
// Layout compatible with WSABUF
struct iovec {
  ULONG iov_len;
  void *iov_base;
};

// Layout compatible with WSAMSG
struct msghdr {
  void         *msg_name;
  socklen_t     msg_namelen;
  struct iovec *msg_iov;
  ULONG         msg_iovlen;
  ULONG         msg_controllen;
  void         *msg_control;
  ULONG         msg_flags;
};
#endif

// External Functions
extern int socket_startup(void);
extern int socket_strerror(int error, char *buffer, size_t buflen);

// Name-Translating Wrappers
#ifndef _WIN32
inline int closesocket(socket_t sock) {
  return close(sock);
}
#endif

inline int last_socket_error(void) {
#ifdef _WIN32
  return WSAGetLastError();
#else
  return errno;
#endif
}

inline int inprogress(int error) {
#ifdef _WIN32
  return (error == WSAEWOULDBLOCK);
#else
  return (error == EWOULDBLOCK) || (error == EAGAIN)
      || (error == EINPROGRESS);
#endif
}

inline int get_cloexec(socket_t sock) {
#ifndef _WIN32
  return fcntl(sock, F_GETFD);
#else
  DWORD flags = 0;
  return GetHandleInformation((HANDLE)sock, & flags)
      ? (flags & HANDLE_FLAG_INHERIT) : -1;
#endif
}

inline int set_cloexec(socket_t sock, int value) {
#ifndef _WIN32
  return fcntl(sock, F_SETFD, value ? FD_CLOEXEC : 0);
#else
  return SetHandleInformation((HANDLE)sock, HANDLE_FLAG_INHERIT,
                              value ? HANDLE_FLAG_INHERIT : 0) ? 0 : -1;
#endif
}

#ifndef _WIN32
inline int set_nonblock(socket_t sock, int value) {
  int oflags = fcntl(sock, F_GETFL);
  if (oflags < 0) return oflags;
  int nflags = value ? (oflags | O_NONBLOCK) : (oflags & ~O_NONBLOCK);
  return (oflags != nflags) ? fcntl(sock, F_SETFL, nflags) : 0;
}
#else
inline int set_nonblock(socket_t sock, u_long value) {
  return ioctlsocket(sock, FIONBIO, &value);
}
#endif

inline socket_t socket4(int domain, int type, int protocol, int flags) {
#ifndef _WIN32
  return socket(domain, type | flags, protocol);
#else
  // Create socket
  socket_t sock = WSASocketW(domain, type, protocol, NULL, 0,
      WSA_FLAG_OVERLAPPED | (flags & SOCK_CLOEXEC));
  if (sock == INVALID_SOCKET) return sock;
  // Apply remaining flags
  u_long arg = 1;
  if (flags & SOCK_NONBLOCK) ioctlsocket(sock, FIONBIO, &arg);
  return sock;
#endif
}

inline int socket_cleanup(void) {
#ifdef _WIN32
  return WSACleanup();
#else
  return 0;
#endif
}

#ifdef _WIN32
inline int poll(struct pollfd *fds, int nfds, int timeout) {
  return WSAPoll(fds, nfds, timeout);
}

inline ssize_t recvmsg(socket_t sock, struct msghdr *msg, DWORD flags) {
  // NOTE: This does not implement the ancillary data feature
  DWORD bytes = 0;
  int result = WSARecvFrom(sock, (WSABUF*)msg->msg_iov, msg->msg_iovlen,
                           &bytes, &flags, (struct sockaddr*)msg->msg_name,
                           &msg->msg_namelen, NULL, NULL);
  if (result == SOCKET_ERROR) return -1;
  msg->msg_flags = flags;
  msg->msg_controllen = 0;
  return (ssize_t)bytes;
}

inline ssize_t sendmsg(socket_t sock, const struct msghdr *msg, DWORD flags) {
  // NOTE: This does not implement the ancillary data feature
  DWORD bytes = 0;
  int result = WSASendTo(sock, (WSABUF*)msg->msg_iov, msg->msg_iovlen,
                         &bytes, flags, (const struct sockaddr*)msg->msg_name,
                         msg->msg_namelen, NULL, NULL);
  if (result == SOCKET_ERROR) return -1;
  return (ssize_t)bytes;
}
#endif

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // SOCKET_H_