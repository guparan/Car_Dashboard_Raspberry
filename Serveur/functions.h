#ifndef ECHANGE_TCP_H
#define ECHANGE_TCP_H

// Libs
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

// CanBus
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>

// Constantes
#define PORT 8080
#define LG_MESSAGE 256
#define CLIENT_MAX 5
#define TAILLE_TRAME 10
#define TAILLE_TRAME_CAN 8
#define TAILLE_INFO_TRAME 3
#define TAILLE_INFO_TRAME_CAN 1


ssize_t ecrireLigne(int sock, const void *buffer, size_t nb);
ssize_t ecrireDonnees(int sock, const void *buffer, size_t nb);

ssize_t lireLigne(int sock, void *buffer, size_t nbMax);
ssize_t lireDonnees(int sock, void *buffer, size_t nbMax);

ssize_t lectureTrame(int liaisonSerie, char * buffer,  size_t nbMax);
ssize_t lectureTrameCan(char *buffer, size_t tailleBuffer);

int saveTrame(FILE* fptr, char *buffer, int sizeofbuffer);
int saveTrameCan(FILE* fptr, char *bufferCan, int sizeofbuffercan);

void convertIntToChar(int value, char* result, int resultSize);

int concatenation(char* frameSerie, char* frameCan,char* tailleTrameSerieLue_encode, char* tailleTrameCanLue_encode);

int initLiaisonSerie();
int initLiaisonCan();

#endif
