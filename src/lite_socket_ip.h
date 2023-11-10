#pragma once
#define MAX_IPADDR_LEN  22 // Including a port
#define IPADDRV4_LEN	15

#include <stdint.h>

typedef struct {
	char addr[MAX_IPADDR_LEN + 1];
	uint16_t port;
}ipv4_t;
