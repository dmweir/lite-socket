#include <stdio.h>
#include "lite_socket.h"


#ifdef __WINDOWS__
#define _CRT_SECURE_NO_WARNINGS 1

static void winsock_init(void);
static void winsock_cleanup(void);

// Initialize Winsock DLL
void winsock_init(void) {
	WSADATA wsaData;
	int err;

	// Winsock version 2.2
	WORD version_required = MAKEWORD(2, 2);

	// https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsastartup
	err = WSAStartup(version_required, &wsaData);
	switch (err) {
		case WSASYSNOTREADY:
			fprintf(stderr, "WSAStartup: Underlying network subsystem is not ready for network communication.\n");
			break;
		case WSAVERNOTSUPPORTED:
			fprintf(stderr, "WSAStartup: The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation.\n");
			break;
		case WSAEINPROGRESS:
			fprintf(stderr, "WSAStartup: A blocking Windows Sockets 1.1 operation is in progress.\n");
	 		break;
		case WSAEPROCLIM:
			fprintf(stderr, "WSAStartup: A limit on the number of tasks supported by the Windows Sockets implementation has been reached.\n");
	 		break;
		case WSAEFAULT:
			fprintf(stderr, "WSAStartup: The lpWSAData parameter is not a valid pointer.\n");
		

	if (err != 0) {
		WSACleanup();
		exit(EXIT_FAILURE);
	}	break;
	}

	// Confirm that winsock DLL supports version 2.2
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		fprintf(stderr, "Could not find a usable version of Winsock.dll\n");
		WSACleanup();
		exit(EXIT_FAILURE);
	}
}

//Cleanup winsock
void winsock_cleanup(void) {
// https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsacleanup
	WSACleanup();
}

void socket_init(void) {
	winsock_init();
}

void socket_cleanup(void) {
	winsock_cleanup();
}

// Retrieve last error code
socket_error_t socket_error(void) {
	// https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-wsagetlasterror
	return WSAGetLastError();
}

// Try to get the error message from the system errors.
void socket_error_detail(socket_error_t error_code, char* msg_buf, int bufsz) {
	// windows: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-formatmessagea
	DWORD nchars = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error_code,
		0,
		(LPSTR)msg_buf,
		bufsz,
		NULL);

	if (nchars == 0) {
		sprintf_s(msg_buf, bufsz, "No message for Error Code %d found.\n", error_code);
	}
}

#endif //__WINDOWS__

#ifdef __UNIX__
void socket_init(void) {
	;
}

void socket_cleanup(void) {
	;
}

// Retrieve last error code
socket_error_t socket_error(void) {
	return errno
}

// Try to get the error message from the system errors.
void socket_error_msg(socket_error_t error_code, char* msg_buf, int bufsz) {
	// linux: https://man7.org/linux/man-pages/man3/strerror.3.html
	char * err_msg = strerror_r(error_code);
	sprintf_s(msg_buf, bufsz, "%s\n", err_msg);
}

#endif //__UNIX__

/* SOCKET FUNCTIONS */
sockfd_t socket_create(socket_type protocol) {
	sockfd_t sockfd = INVALID_SOCKET;

	switch (protocol) {
	case TCP:
			sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			break;
	case UDP:
			sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			break;
	}
	return sockfd;
}

socket_error_t socket_connect(sockfd_t sockfd, const ipv4_t* server_addr) {
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	inet_pton(AF_INET, server_addr->addr, &server.sin_addr.s_addr);
	server.sin_port = htons(server_addr->port);

	// windows: https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-connect
	// linux: https://man7.org/linux/man-pages/man2/connect.2.html
	if (connect(sockfd, (struct sockaddr*) & server, sizeof(server)) == SOCKET_ERROR) {
		return socket_error();
	}
	return SOCKET_OK;
}

socket_error_t socket_bind(sockfd_t sockfd, const ipv4_t* addr) {
	struct sockaddr_in service;
	service.sin_family = AF_INET;
	inet_pton(AF_INET, addr->addr, &service.sin_addr.s_addr);
	service.sin_port = htons(addr->port);

	// windows: https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-bind
	// linux: https://man7.org/linux/man-pages/man2/bind.2.html
	if (bind(sockfd, (const struct sockaddr*)&service, sizeof(service)) == SOCKET_ERROR) {
		int bind_err = socket_error();
		socket_close(sockfd);
		return bind_err;
	}
	return SOCKET_OK;
}

socket_error_t socket_listen(sockfd_t sockfd, int backlog) {
	// windows: https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-listen
	// linux: https://man7.org/linux/man-pages/man2/listen.2.html
	int max_pending = backlog > SOMAXCONN ? SOMAXCONN : backlog;
	if (listen(sockfd, max_pending) == SOCKET_ERROR) {
		return socket_error();
	}
	return SOCKET_OK;
}

sockfd_t socket_accept(sockfd_t sockfd, socket_error_t* error) {
	struct sockaddr_in incoming;
	socklen_t addrlen = sizeof(incoming);

	// windows: https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-accept
	// linux: https://man7.org/linux/man-pages/man2/accept.2.html
	sockfd_t connfd = accept(sockfd, (struct sockaddr*) & incoming, &addrlen);

	if (connfd == INVALID_SOCKET) {
		*error = socket_error();
		return connfd;
	}

	*error = SOCKET_OK;
	return connfd;
}

socket_error_t socket_getname(sockfd_t sock, ipv4_t* addr) {
	// windows: https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-getsockname
	// linux: https://man7.org/linux/man-pages/man2/getsockname.2.html
	struct sockaddr_in name;
	socklen_t namelen = sizeof(name);
	if (getsockname(sock, (struct sockaddr*)&name,  &namelen) == SOCKET_ERROR) {
		return socket_error();
	}

	addr->port = ntohs(name.sin_port);

	int res = inet_ntop(AF_INET, &(name.sin_addr), &(addr->addr[0]), sizeof(addr->addr));
	if (res == 0) {
		fprintf(stderr, "Not in presentation format");
		exit(EXIT_FAILURE);
	}
	else if (res < 0) {
		return socket_error();
	}


	return SOCKET_OK;
}

socket_error_t socket_shutdown(sockfd_t sockfd) {
	//how: SD_SEND, SD_RECV, SD_BOTH
	// windows: https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-shutdown
	// linux: https://man7.org/linux/man-pages/man2/shutdown.2.html
	if (shutdown(sockfd, SD_BOTH) == SOCKET_ERROR) {
		return socket_error();
	}
	return SOCKET_OK;
}

socket_error_t socket_close(sockfd_t sockfd) {
	if (closesocket(sockfd) == SOCKET_ERROR) {
		return socket_error();
	}
	return SOCKET_OK;
}

int socket_recv(sockfd_t sockfd,  void *buf, int len, int flags) {
	// Note: User must check the error code if SOCKET_ERROR is returned.
	// Returns the number of bytes read or SOCKET_ERROR
	// If the connection has been gracefully closed, the return value is zero.
	int nbytes = recv(sockfd, buf, len, flags);
	if (nbytes == SOCKET_ERROR)
		return SOCKET_ERROR;
	else if (nbytes == 0) {
		return 0;
	}
	else
		return nbytes;
}

int socket_send(sockfd_t sockfd, void *buf, int len, int flags) {
	// Note: User must check the error code if SOCKET_ERROR is returned.
	// Returns the number of bytes sent or SOCKET_ERROR
	// If space is not available at the sending socket to hold the message 
	// to be transmitted, and the socket file descriptor does not have O_NONBLOCK set,
	// send() shall block until space is available.
	// If space is not available at the sending socket to hold the message 
	// to be transmitted, and the socket file descriptor does have O_NONBLOCK set, send() shall fail.
	int nbytes = send(sockfd, buf, len, flags);
	if (nbytes == SOCKET_ERROR)
		return SOCKET_ERROR;
	else
		return nbytes;
}

int socket_sendall(sockfd_t sockfd, void* buf, int len, int flags) {
	//flags: MSG_OOB, MSG_DONTROUTE
	const char* b = (const char*)buf;
	int bytes_sent = 0;
	int buf_len = len;

	while (bytes_sent < len) {
		int res = send(sockfd, b, buf_len, flags);
		if (res <= 0) {
			return res;
		}

		bytes_sent += res;
		b += bytes_sent;
		buf_len -= bytes_sent;
	}

	return bytes_sent;
}



socket_error_t socket_setsockopt(sockfd_t sockfd, int level, int optname, int *optval, socklen_t optlen) {
	// windows: https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-setsockopt
	// linux: https://man7.org/linux/man-pages/man3/setsockopt.3p.html
	if (setsockopt(sockfd, level, optname, OPTVAL_CAST(optval), optlen) == SOCKET_ERROR) {
		return socket_error();
	}
	return SOCKET_OK;
}

socket_error_t socket_getsockopt(sockfd_t sockfd, int level, int optname, int *optval, socklen_t* optlen) {
	// windows: https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-getsockopt
	// linux: https://man7.org/linux/man-pages/man2/getsockopt.2.html
	if (getsockopt(sockfd, level, optname, OPTVAL_CAST(optval), optlen) == SOCKET_ERROR) {
		return socket_error();
	}
	return SOCKET_OK;
}

socket_error_t socket_tcp_nodelay(sockfd_t sockfd, int optval) {
	return socket_setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

socket_error_t socket_reuseaddr(sockfd_t sockfd, int optval) {
	return socket_setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

//Print an error message
void socket_error_print(const char* func_name, socket_error_t error_code) {
	char err_msg[512];
	if (func_name != NULL) {
		fprintf(stderr, "%s: ", func_name);
	}
	socket_error_detail(error_code, err_msg, sizeof(err_msg));
	fprintf(stderr, "%s", err_msg);
}

int socket_read_ready(sockfd_t sock, double timeout) {
	struct timeval wait, * pWait;
	if (timeout < 0) { // Negative timeout value means we are willing to wait forever
		pWait = NULL;
	}
	else {
		wait.tv_sec = (long)timeout;
		double remainder = timeout - ((double)(wait.tv_sec));
		wait.tv_usec = (long)(remainder * 1000000.0);
		pWait = &wait;
	}

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sock, &readfds);
	int nfds = sock + 1;

	int status = select(nfds, &readfds, NULL, NULL, pWait);

	if (status < 0) {
		return socket_error();
	}
	return status;
}

int socket_write_ready(sockfd_t sock, double timeout) {
	struct timeval wait, * pWait;
	if (timeout < 0) { // Negative timeout value means we are willing to wait forever
		pWait = NULL;
	}
	else {
		wait.tv_sec = (long)timeout;
		double remainder = timeout - ((double)(wait.tv_sec));
		wait.tv_usec = (long)(remainder * 1000000.0);
		pWait = &wait;
	}

	fd_set writefds;
	FD_ZERO(&writefds);
	FD_SET(sock, &writefds);
	int nfds = sock + 1;

	int status = select(nfds, NULL, &writefds, NULL, pWait);

	if (status < 0) {
		return socket_error();
	}
	return status;
}

socket_error_t socket_set_nonblocking(sockfd_t sock, int value) {
//Set non-blocking I/O mode if the argument is non-zero

#ifdef __WINDOWS__
	if (ioctlsocket(sock, FIONBIO, (u_long*)&value) == SOCKET_ERROR) {
		return socket_error();
	}
	return SOCKET_OK;
#else
	if (ioctl(sock, FIONBIO, (u_long*)&value) == SOCKET_ERROR) {
		return socket_error();
	}
	return SOCKET_OK;
#endif
}

socket_error_t socket_set_recv_timeout(sockfd_t sockfd, int timeout_ms) {
	return socket_setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout_ms, sizeof(timeout_ms));
}

socket_error_t socket_set_send_timeout(sockfd_t sockfd, int timeout_ms) {
	return socket_setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout_ms, sizeof(timeout_ms));
}
