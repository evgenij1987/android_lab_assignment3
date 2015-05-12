/*
 * udpserver.c
 *
 *  Created on: May 7, 2015
 *      Author: evgenijavstein
 */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "udpserver.h"
#include "helper.h"


//we store all registered clients here
//all unregistered are kicked out as soon as one new client is registering
Client * clients[MAX_CLIENTS];

int main(int argc, char *argv[]) {

	if (argc != REQUIRED_NUMBER_OF_CMD_ARGUMENTS)
		dieGracefully("Port was not provided by the user");

	char *port = argv[1];

	//create udp socket on specified port
	int datagramSocket = createDatagramSocket(port);
	printServeringRunningOn(port);

	for (;;) {

		// Client address AF-independent, can hold both AF_INET & AF_INET6 adresses
		struct sockaddr_storage clntAddr;
		// Set Length of client address structure (in-out parameter)
		socklen_t clntAddrLen = sizeof(clntAddr);

		char buffer[MAX_MESSAGESIZE];

		ssize_t numBytesRcvd = recvfrom(datagramSocket, buffer, MAX_MESSAGESIZE, 0, (struct sockaddr *) &clntAddr, &clntAddrLen);
		if (numBytesRcvd < 0)
			dieGracefully("recvfrom() failed");

		fputs("Handling client ", stdout);
		printSocketAdress((struct sockaddr *) &clntAddr, stdout);
		fputc('\n', stdout);

		if (buffer[0] == TYPE_REGISTER) {

			//allocate new client & add to list
			Client * clnt = malloc(sizeof(Client));
			clnt->clnt_addr = clntAddr;
			clnt->last_keep_alive_request = time(0);
			clnt->name_len = buffer[1];
			clnt->name = malloc(clnt->name_len);
			strncpy(clnt->name, (char *) buffer + 2, clnt->name_len);

			addClient(clnt);
		} else if (buffer[0] == TYPE_KEEPALIVE) {
			///client stored in the list is retrieved to receive a fresh timestamp
			Client * existingClient = findClientBySocktAddr(&clntAddr);
			if (existingClient)
				existingClient->last_keep_alive_request = time(0); //keep client alive for next 20 seconds

		} else if (buffer[0] == TYPE_EVENT) {
			//EVENT, translated to shake, is sent to all registered clients
			Event * event = (Event*) buffer;
			sendToAllRegistered(datagramSocket, &clntAddr, event);
		} else if (buffer[0] == TYPE_UNREGISTER) {
			Client * unregisteringClient = findClientBySocktAddr(&clntAddr);
			free(unregisteringClient);
		}

	}
//NEVER REACHED
}
/**
 * Method checks for available slots within clients array.
 * Available slot is NULL or a client, which didn't sent keep alive for
 * longer then 20 seconds. If array contains another client with same socket adress the client
 * is overriden with the new one, this way there are no dublicates and client can update their name.
 */
void addClient(Client * clnt) {
	int i;
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (clients[i] == NULL || !isRegistered(clients[i]) || isSameClient(clnt, clients[i])) {
			//free(clients[i]);//free client which is no longer valid
			clients[i] = clnt;
			return;
		}

	}

}

bool isRegistered(Client * client) {
	return ((time(0) - client->last_keep_alive_request) <= 20);
}
bool isSameClient(Client * new, Client*in_list) {

	return sockAddressEquals((struct sockaddr *) &(new->clnt_addr), (struct sockaddr *) &in_list->clnt_addr);
}

Client *findClientBySocktAddr(struct sockaddr_storage * sock_adrr) {
	int i;

	for (i = 0; i < MAX_CLIENTS; i++) {
		if (clients[i] != NULL)
			if (sockAddressEquals((struct sockaddr *) &(clients[i]->clnt_addr), (struct sockaddr *) sock_adrr))
				return clients[i];
	}
	return NULL;
}

/**
 * Sends Shake packet created from <code>event</code> to all registered, except the sender itself.
 */
void sendToAllRegistered(int sock, struct sockaddr_storage * senderAddr, Event* event) {

	Client * sender = findClientBySocktAddr(senderAddr);
	if (sender && isRegistered(sender)) { //only if sender is registered handled
		Shake * shake;
		//char * name =sender->name;
		//size_t name_len=sender->name_len;
		size_t shake_header_size = sizeof(Shake);
		size_t buffer_size = shake_header_size + sender->name_len;
		char * buffer = malloc(buffer_size);
		shake = (Shake*) buffer;
		shake->type = TYPE_SHAKE;
		shake->timestamp = event->timestamp;			//already in Big Endian
		shake->len_name = sender->name_len;

		char * payload = buffer + shake_header_size;
		strncpy(payload, sender->name, sender->name_len);

		int i;
		for (i = 0; i < MAX_CLIENTS; i++) {
			Client * clnt = clients[i];

			//send events only to those still registered
			//send to all clients except the current sender
			if ((clnt != NULL) && !isSameClient(sender, clnt) && isRegistered(clnt)) {

				ssize_t numBytesSent = sendto(sock, buffer, buffer_size, 0, (struct sockaddr *) &clnt->clnt_addr,
						sizeof(clnt->clnt_addr));

				if (numBytesSent < 0)
					dieGracefully("sendto() failed)");
				else if (numBytesSent != buffer_size)
					dieGracefully("sendto():", "sent unexpected number of bytes");

				fputs("Event sent to:", stdout);
				printSocketAdress((struct sockaddr *) &clnt->clnt_addr, stdout);
				fputc('\n', stdout);

			}
		}
		free(buffer);

	}

}

