#ifndef ECHANGE_TCP_H
#define ECHANGE_TCP_H

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <termios.h>
#include <sys/fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

ssize_t ecrireLigne(int sock, const void *buffer, size_t nb);
ssize_t ecrireDonnees(int sock, const void *buffer, size_t nb);

ssize_t lireLigne(int sock, void *buffer, size_t nbMax);
ssize_t lireDonnees(int sock, void *buffer, size_t nbMax);

ssize_t lectureTrame(int liaisonSerie, char * buffer,  size_t nbMax);
ssize_t lectureTrameCan(char *buffer, size_t tailleBuffer);

int saveTrame(FILE* fptr, char *buffer, int j, int sizeofbuffer);
int saveTrameCan(FILE* fptr, char *bufferCan, int j, int sizeofbuffercan);

void convertIntToChar(int value, char* result, int resultSize);

int concatenation(char* frameSerie, char* frameCan,char* tailleTrameSerieLue_encode, char* tailleTrameCanLue_encode);


#endif
