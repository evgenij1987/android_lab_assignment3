/*
 * udpserver.h
 *
 *  Created on: May 7, 2015
 *      Author: evgenijavstein
 */

#ifndef UDPSERVER_H_
#define UDPSERVER_H_
#define MAX_MESSAGESIZE 255
#define REQUIRED_NUMBER_OF_CMD_ARGUMENTS 2
#define TYPE_REGISTER 1
#define TYPE_UNREGISTER 2
#define TYPE_KEEPALIVE 3
#define TYPE_EVENT 4
#define TYPE_SHAKE 5
#define MAX_CLIENTS 20
#define KEEP_ALIVE_TIME 10

uint64_t get_time_stamp();
typedef struct {
	uint8_t type;
	uint64_t timestamp;
}__attribute__ ((__packed__)) Event;

typedef struct {
	uint8_t type;
	uint64_t timestamp;
	uint8_t len_name;

}__attribute__ ((__packed__)) Shake;




typedef struct {
	char * name;
	size_t name_len;
	struct sockaddr_storage  clnt_addr;
	time_t last_keep_alive_request;

}Client;

Client *findClientBySocktAddr(struct sockaddr_storage * sock_adrr);
void addClient(Client * clnt);
void  findClientMarkRemoved(struct sockaddr_storage * sock_adrr);
void sendToAllRegistered(int sock, struct sockaddr_storage * senderAddr, Event* event);
bool isRegistered(Client * client);
bool isSameClient(Client * new,Client*in_list );

#endif /* UDPSERVER_H_ */
