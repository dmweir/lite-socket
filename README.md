# lite-socket

A C library that supports ipv4 tcp/udp sockets on windows and unix.

Provides basic functions for common operations and helpers for returning system error messages. 

## Usage

### Direct

Copy the `src\` folder into your project.

### Compile to Shared or Static library

From the project root directory:

```
mkdir build
cd build
cmake ..
```

### Example

```C
#include "lite_socket.h"
#include <stdint.h>

int main(void) {
	socket_error_t  err;
	socket_init();
	sockfd_t sock = socket_create(TCP);

	if (sock == INVALID_SOCKET) {
		socket_error_print("socket_create", socket_error());
		exit(EXIT_FAILURE);
	}

	err = socket_tcp_nodelay(sock, 1);
	if (err) {
		socket_error_print("socket_tcp_nodelay", err);
		exit(EXIT_FAILURE);
	}

	int optval = 0;
	socklen_t optlen = sizeof(optval);
	err = socket_getsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &optval, &optlen);
	if (err) {
		socket_error_print("socket_getsockopt", err);
		exit(EXIT_FAILURE);
	}


	err = socket_bind(sock, &(ipv4_t) { "127.0.0.1", 7111 });
	if (err) {
		socket_error_print("socket_bind", err);
		exit(EXIT_FAILURE);
	}

	ipv4_t name;

	err = socket_getname(sock, &name);
	if (err != SOCKET_OK) {
		socket_error_print("socket_getname", err);
	}
	else {
		printf("%s:%d, tcp_delay=%d\n", name.addr, name.port, optval);
	}

	err = socket_close(sock);
	if (err) {
		socket_error_print("socket_bind", err);
	}

	socket_cleanup();

	return 0;
}
```
