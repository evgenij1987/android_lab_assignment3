#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/types.h>
#include <ifaddrs.h>

void printSocketAdress(const struct sockaddr *address, FILE *stream) {
	// Test for address and stream
	if (address == NULL || stream == NULL)
		return;

	void *numericAddress; // Pointer to binary address
	// Buffer to contain result (IPv6 sufficient to hold IPv4)
	char addrBuffer[INET6_ADDRSTRLEN];
	in_port_t port; // Port to print
	// Set pointer to address based on address family
	switch (address->sa_family) {
	case AF_INET:
		numericAddress = &((struct sockaddr_in *) address)->sin_addr;
		port = ntohs(((struct sockaddr_in *) address)->sin_port);
		break;
	case AF_INET6:
		numericAddress = &((struct sockaddr_in6 *) address)->sin6_addr;
		port = ntohs(((struct sockaddr_in6 *) address)->sin6_port);
		break;
	default:
		fputs("[unknown type]", stream);    // Unhandled type
		return;
	}
	// Convert binary to printable address
	if (inet_ntop(address->sa_family, numericAddress, addrBuffer, sizeof(addrBuffer)) == NULL)
		fputs("[invalid address]", stream); // Unable to convert
	else {
		fprintf(stream, "%s", addrBuffer);
		if (port != 0)                // Zero not valid in any socket addr
			fprintf(stream, "-%u", port);
	}
}

bool sockAddressEquals(const struct sockaddr *addr1, const struct sockaddr *addr2) {
	if (addr1 == NULL || addr2 == NULL)
		return addr1 == addr2;
	else if (addr1->sa_family != addr2->sa_family)
		return false;
	else if (addr1->sa_family == AF_INET) {
		struct sockaddr_in *ipv4Addr1 = (struct sockaddr_in *) addr1;
		struct sockaddr_in *ipv4Addr2 = (struct sockaddr_in *) addr2;
		return ipv4Addr1->sin_addr.s_addr == ipv4Addr2->sin_addr.s_addr && ipv4Addr1->sin_port == ipv4Addr2->sin_port;
	} else if (addr1->sa_family == AF_INET6) {
		struct sockaddr_in6 *ipv6Addr1 = (struct sockaddr_in6 *) addr1;
		struct sockaddr_in6 *ipv6Addr2 = (struct sockaddr_in6 *) addr2;
		return memcmp(&ipv6Addr1->sin6_addr, &ipv6Addr2->sin6_addr, sizeof(struct in6_addr)) == 0
				&& ipv6Addr1->sin6_port == ipv6Addr2->sin6_port;
	} else
		return false;
}

int createDatagramSocket(char* port) {
	// Construct the server address structure
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	struct addrinfo* servAddr;
	// List of server addresses

	int rtnVal = getaddrinfo(NULL, port, &hints, &servAddr);
	if (rtnVal != 0)
		dieGracefully("getaddrinfo() failed: %s.", gai_strerror(rtnVal));

	// Create socket for incoming connections
	int sock = socket(servAddr->ai_family, servAddr->ai_socktype, servAddr->ai_protocol);
	if (sock < 0)
		dieGracefully("socket() failed");

	// Bind to the local address
	if (bind(sock, servAddr->ai_addr, servAddr->ai_addrlen) < 0)
		dieGracefully("bind() failed");

	fputc('\n', stdout);
	// Free address list allocated by getaddrinfo()
	freeaddrinfo(servAddr);
	return sock;
}

void printServeringRunningOn(char * port) {
	struct ifaddrs *ifap, *ifa;
	struct sockaddr_in *sa;
	char *addr;

	getifaddrs(&ifap);
	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family == AF_INET) {
			sa = (struct sockaddr_in *) ifa->ifa_addr;
			addr = inet_ntoa(sa->sin_addr);

			if (strcmp(ifa->ifa_name, "eth0") == 0)
				printf("Sensor server running on: %s %s \n", addr, port);

		}
	}

	freeifaddrs(ifap);
}

void dieGracefully(const char *format, ...) {
	va_list arg;
	va_start(arg, format);
	vfprintf(stderr, format, arg);
	va_end(arg);
	exit(1);
}
