#ifndef _LITE_SOCKET_H
#define _LITE_SOCKET_H

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
#define __WINDOWS__
#else
#define __UNIX__
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#ifdef __WINDOWS__

#undef UNICODE
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>

#define _getpid getpid

typedef SOCKET sockfd_t;
typedef int sa_family_t;
typedef int socklen_t;

#pragma comment(lib, "Ws2_32.lib")

#else // __UNIX__

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket(x) close(x)

typedef int sockfd_t;

#define SD_BOTH SHUT_RDWR
#define SD_SEND SHUT_WR
#define SD_RECEIVE SHUT_RD

#endif

#include <stdint.h>

#ifdef __WINDOWS__
#ifdef USE_DYNAMIC_LIB
#ifdef LITE_SOCKET_C_EXPORTS
#define LITE_SOCKET_C_API __declspec(dllexport)
#else
#define LITE_SOCKET_C_API __declspec(dllimport)
#endif
#else
#define LITE_SOCKET_C_API
#endif
#elif defined(__UNIX__) || defined(__APPLE__) || defined(__MACH__) || defined(__LINUX__) || defined(__FreeBSD__)
#define LITE_SOCKET_C_API //nothing
#else
#define LITE_SOCKET_C_API //nothing
#endif

#ifdef __WINDOWS__
#define OPTVAL_CAST(x) (char *)(x)
#else
#define OPTVAL_CAST(x) (void *)(x)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	TCP = 1,
	UDP = 2,
}socket_type;

typedef struct {
	char ip[INET_ADDRSTRLEN + 1];
	uint16_t port;
}ipaddress_t;


/* SOCKET FUNCTIONS */
#define SOCKET_OK 0
typedef int socket_error_t;

LITE_SOCKET_C_API void socket_init(void);
LITE_SOCKET_C_API void socket_cleanup(void);

LITE_SOCKET_C_API sockfd_t socket_create(socket_type protocol);
LITE_SOCKET_C_API socket_error_t socket_connect(sockfd_t sock, const ipaddress_t* server_addr);
LITE_SOCKET_C_API socket_error_t socket_bind(sockfd_t sockfd, const ipaddress_t* addr);
LITE_SOCKET_C_API socket_error_t socket_getname(sockfd_t sock, ipaddress_t* addr);
LITE_SOCKET_C_API socket_error_t socket_listen(sockfd_t sockfd, int backlog);
LITE_SOCKET_C_API sockfd_t socket_accept(sockfd_t sockfd, socket_error_t* error);
LITE_SOCKET_C_API socket_error_t socket_close(sockfd_t sockfd);
LITE_SOCKET_C_API socket_error_t socket_shutdown(sockfd_t sockfd);
LITE_SOCKET_C_API int socket_recv(sockfd_t sockfd, void* buf, int len, int flags);
LITE_SOCKET_C_API int socket_send(sockfd_t sockfd, void* buf, int len, int flags);
LITE_SOCKET_C_API int socket_sendall(sockfd_t sockfd, void* buf, int len, int flags);
LITE_SOCKET_C_API int socket_read_ready(sockfd_t sock, double timeout);
LITE_SOCKET_C_API int socket_write_ready(sockfd_t sock, double timeout);
LITE_SOCKET_C_API socket_error_t socket_setsockopt(sockfd_t sockfd, int level, int optname, int* optval, socklen_t optlen);
LITE_SOCKET_C_API socket_error_t socket_getsockopt(sockfd_t sockfd, int level, int optname, int* optval, socklen_t* optlen);

/* HELPER FUNCTIONS */
LITE_SOCKET_C_API socket_error_t socket_error(void);
LITE_SOCKET_C_API void socket_error_print(const char* func_name, socket_error_t error_code);
LITE_SOCKET_C_API void socket_error_msg(socket_error_t error_code, char* msg_buf, int bufsz);
LITE_SOCKET_C_API double get_timestamp(void);

#ifdef __cplusplus
}
#endif

#endif //_LITE_SOCKET_H
