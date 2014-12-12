// URL : http://collabedit.com/7fpwk

// Bibliotheques
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <termios.h>
#include <sys/fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

//CanBus
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>

// Fonctions
#include "functions.h"

// Constantes
#define PORT 8080
#define LG_MESSAGE 256
#define CLIENT_MAX 5
#define TAILLE_TRAME 10
#define TAILLE_TRAME_CAN 8
#define TAILLE_INFO_TRAME 3
#define TAILLE_INFO_TRAME_CAN 1


static int keepRunning = 1;


void intHandler(int sig)
{
    printf("signal recupere %d\n", sig);
    keepRunning=0;
    exit(sig);
}


void *thread_runtime (void * arg)
{
    int * clients=(int *)arg;
	
	int tailleTrameClient = TAILLE_TRAME+TAILLE_TRAME_CAN+TAILLE_INFO_TRAME+TAILLE_INFO_TRAME_CAN;
	char* trameClient = (char*)malloc(tailleTrameClient);
	char* tailleTrameSerieLue_char = trameClient;
	char* tailleTrameCanLue_char = tailleTrameSerieLue_char + TAILLE_INFO_TRAME;
	char* buffer = tailleTrameCanLue_char + TAILLE_INFO_TRAME_CAN;
	char* bufferCan = buffer + TAILLE_TRAME

    //char* bufferTest="Testbuffer";
	char* tailleTrameSerieLue_buffer;

    int fdSerie;
	int fdCan;
	
    int trame;
    int save;
    int i;
    int j=0;
    int ecrits=0;
	int digits = 0;
	
	// TESTS
	int tailleTrameCanLue_int = 8;
	for(i=0 ; i<8 ; i++)
	{
		bufferCan[i] = (char)i+40;
	}
	
	//int tailleTrameCanLue_int = 0;	
	int tailleTrameSerieLue_int = 0;
	
	printf("thread cree\n");

	FILE* logSerie = fopen("data.csv", "w");
	FILE* logCan = fopen("dataCAN.csv", "w");
	
	fdSerie = initLiaisonSerie();
	//fdCan = initLiaisonCan();
	
    printf("keepRunning %d\n", keepRunning);

    while ( keepRunning )
    {
		// LECTURE TRAME CAN
		//tailleTrameCanLue_int = lectureTrameCan(fdCan, bufferCan, TAILLE_TRAME_CAN));
		if( tailleTrameCanLue_int == 0 )
		{
			// error
		}
		if( saveTrameCan(logCan, bufferCan, TAILLE_TRAME_CAN) ) printf("la sauvegarde CAN a bien ete faite \n");
		convertIntToChar(tailleTrameCanLue_int, tailleTrameCanLue_char, TAILLE_INFO_TRAME_CAN);
		
		// LECTURE TRAME SERIE
		tailleTrameSerieLue_int = lectureTrame(fdSerie, buffer, TAILLE_TRAME);
        if( tailleTrameSerieLue_int == 0 )
		{
			// error
		}
        if( saveTrame(logSerie, buffer, TAILLE_TRAME) ) printf("la sauvegarde SERIE a bien ete faite \n");
		convertIntToChar(tailleTrameSerieLue_int, tailleTrameSerieLue_char, TAILLE_INFO_TRAME);		

        for(i=0 ; i<CLIENT_MAX ; i++)
        {
            //printf("check client %d\n", i);
            if(clients[i]==-1)
            {
                //printf("Pas de client a %d\n", i);
                continue;
            }
            printf("tentative decriture sur le client %d \n", i);
            ecrits = write(clients[i], trameClient, tailleTrameClient);
            printf("code de retour du write : %d \n", ecrits);

            if(ecrits == -1)
            {
                printf("Error writing from socket ! \n");
                printf("errno = %d \n", errno);

                if(errno == EPIPE || errno == ECONNRESET)
                {
                    printf("deconnexion client\n");
                    close(clients[i]);
                    clients[i]=-1;
                }
				else
				{
					printf("Erreur inconnue \n");
					exit(errno);
				}
            }
			else
			{
				printf("Send messages finished:%s (%d octets)!\n", buffer, ecrits);
			}
		}

        //printf("%03u %02x %c\n",c&0xff,c&0xff,c);
    }

    // Ce code est atteint si keepRunning == 0
    for(i=0 ; i<CLIENT_MAX ; i++)
    {
        if(clients[i] != -1)
        {
            close(clients[i]);
            clients[i] = -1;
        }
    }
    close(fdSerie);
    close(fdCan);
	
    printf("fin du thread\n");
    return 0;
}



int main()
{
    pthread_t thread;
    pthread_t threadCan;
    int socketServeur;
    int socketClient;
    int i;
    int clients[CLIENT_MAX];
    struct sockaddr_in addrServeur;
    socklen_t longueurAdresse;
    char hbuf[1024], sbuf[32];

    // Gestion du signal d'interuption Ctrl+C
    signal(SIGINT, intHandler);
    signal(SIGPIPE, SIG_IGN);
    // Initialisation du tableau des clients
    for(i=0 ; i<CLIENT_MAX ; i++)
    {
        clients[i]=-1;
    }

    // Crée un socket de communication
    socketServeur = socket(PF_INET, SOCK_STREAM, 0);//protocole par défaut
    if(socketServeur == -1)
    {
        perror("Socket");
        exit(-1);
    }

    printf("Socket crée avec succès ! (%d)\n", socketServeur);

    addrServeur.sin_addr.s_addr = INADDR_ANY; //toutes
    addrServeur.sin_family = PF_INET;
    addrServeur.sin_port = htons(PORT);

    // Demande l'attachement local de la socket
    longueurAdresse = sizeof(addrServeur);
    if( bind(socketServeur, (struct sockaddr *)&addrServeur, longueurAdresse) == -1 )
    {
        perror("bind");
        exit(-2);
    }
    printf("Socket attachée avec succès!\n");

    if (listen(socketServeur, CLIENT_MAX) == -1)
    {
        perror("listen");
        exit(errno);
    }
    printf("Socket placée en écoute passive...\n");

    //memset(buffer,0x00,LG_MESSAGE*sizeof(char));
    pthread_create(&thread, NULL, thread_runtime, clients);
    printf("creation du thread\n");

    pthread_create(&threadCan, NULL, thread_runtimeCan, clients);
    printf("creation du threadCan\n");

    while(1)
    {
        printf("Attente d'une demande de connexion (quitter avec Cltrl-C)\n\n");

        socketClient = accept(socketServeur, (struct sockaddr *)&addrServeur, &longueurAdresse);
        if(socketClient == -1 )
        {
            perror("accept");
            close(socketClient);
            close(socketServeur);
            exit(errno);
        }

        printf("Nouveau client !\n");

        for(i=0 ; i<CLIENT_MAX ; i++)
        {
            if(clients[i]==-1)
            {
                clients[i]=socketClient;
                printf("Client assigne a l'indice %d\n", i);
                break;
            }
        }

        if(i==CLIENT_MAX)
        {
            printf("Table des client pleine, client rejete !\n");
            close(socketClient);
        }

        if ( getnameinfo((struct sockaddr*)&addrServeur, sizeof(addrServeur), hbuf, sizeof(hbuf), sbuf,
                       sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV) == 0)
        {
               printf("client=%s, port=%s\n", hbuf, sbuf);
        }
        else
        {
            printf("Marche pas\n");
            //printf("host=%s, serv=%s\n", hbuf, sbuf);
        }
    }
    // Attente de la fin du thread
    if(pthread_join(thread, NULL) !=0)
    {
        perror("pthread_join");
        exit(errno);
    }
    for(i=0 ; i<CLIENT_MAX ; i++)
    {
        if(clients[i] != -1)
        {
            close(clients[i]);
            clients[i] = -1;
        }
    }
    close(socketServeur);
    return 0;
}
