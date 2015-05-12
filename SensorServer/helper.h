/*
 * adresshelper.h
 *
 *  Created on: May 7, 2015
 *      Author: evgenijavstein
 */
#include <stdbool.h>
#ifndef ADRESSHELPER_H_
#define ADRESSHELPER_H_

void printSocketAdress(const struct sockaddr *address, FILE *stream);
bool sockAddressEquals(const struct sockaddr *addr1, const struct sockaddr *addr2);
void dieGracefully(const char *format, ...);
bool sockAddressEquals(const struct sockaddr *addr1, const struct sockaddr *addr2);
int createDatagramSocket(char* port);
void printServeringRunningOn();
#endif /* ADRESSHELPER_H_ */
