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

// Fonctions
#include "functions.h"

// Constantes
#define PORT 8080
#define LG_MESSAGE 256
#define CLIENT_MAX 5
#define TAILLE_TRAME 10

//CanBus
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>

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

    char buffer[TAILLE_TRAME];
    //char* bufferTest="Testbuffer";

    int liaisonSerie;
    int trame;
    int save;
    int i;
    int j=0;
    int ecrits=0;
    struct termios termios_p;

    FILE* fptr = fopen("data.csv", "w");

    printf("thread cree\n");

    if ( (liaisonSerie = open("/dev/ttyAMA0",O_RDONLY)) == -1 )
    {
        printf("error on open");
        exit(-1);
    }

    /* Lecture des parametres courants */
    tcgetattr(liaisonSerie,&termios_p);
    /* On ignore les BREAK et les caracteres avec erreurs de parite */
    termios_p.c_iflag = IGNBRK | IGNPAR;
    /* Pas de mode de sortie particulier */
    termios_p.c_oflag = 0;
    /* Liaison a 9600 bps avec 7 bits de donnees et une parite paire */
    termios_p.c_cflag = B9600 | CS7 | PARENB;
    /* Mode non-canonique avec echo */
    termios_p.c_lflag = ECHO;
    /* Caracteres immediatement disponibles */
    termios_p.c_cc[VMIN] = 1;
    termios_p.c_cc[VTIME] = 0;
    /* Sauvegarde des nouveaux parametres */
    tcsetattr(liaisonSerie,TCSANOW,&termios_p);


    printf("keepRunning %d\n", keepRunning);

    while ( keepRunning )
    {
        trame = lectureTrame(liaisonSerie, buffer, TAILLE_TRAME);
        if(trame == 1) printf("la trame est correct\n");
        printf("%s\n", buffer);
        printf("%c\n", buffer[0]);

        save = saveTrame(fptr, buffer, j, TAILLE_TRAME);
        j++;
        if(save == 1) printf("la sauvegarde a bien ete faite \n");

     // system("gnuplot gnuplot_config");

        for(i=0 ; i<CLIENT_MAX ; i++)
        {
            //printf("check client %d\n", i);
            if(clients[i]==-1)
            {
                //printf("Pas de client a %d\n", i);
                continue;
            }
            printf("tentative decriture sur le client %d \n", i);
            ecrits = write(clients[i], buffer, TAILLE_TRAME);
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
    close(liaisonSerie);
    printf("fin du thread\n");

    for(i=0 ; i<CLIENT_MAX ; i++)
    {
        if(clients[i] != -1)
        {
            close(clients[i]);
            clients[i] = -1;
        }
    }

    return 0;
}

void *thread_runtimeCan (void * arg)
{
        printf("je suis dans le thread tramecan\n");
/*
        int s,i;
        int nbytes;
        struct sockaddr_can addr;
        struct can_frame frame;
        struct ifreq ifr;
	char c;
	char buffer[100];

        char *ifname = "can0";

        if((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
                perror("Error while opening socket");
                return -1;
        }
        printf("socket canbus cree avec sucees\n");
        strcpy(ifr.ifr_name, ifname);
        ioctl(s, SIOCGIFINDEX, &ifr);

        addr.can_family  = AF_CAN;
        addr.can_ifindex = ifr.ifr_ifindex;

        printf("%s at index %d\n", ifname, ifr.ifr_ifindex);

        if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                perror("Error in socket bind");
                return -2;
        }
        printf("socket attache avec succes\n");

        frame.can_id  = 0x123;
        frame.can_dlc = 2;
        frame.data[0] = 0xB0;
        frame.data[1] = 0x0B;

    nbytes = write(s, &frame, sizeof(struct can_frame));
	printf("Wrote %d bytes\n", nbytes);

        frame.can_id  = 0x456;
        frame.can_dlc = 2;
        frame.data[0] = 0x13;
        frame.data[1] = 0x37;

	nbytes = write(s, &frame, sizeof(struct can_frame));
    printf("Wrote %d bytes\n", nbytes);

    while(keepRunning)
    {
      //   Read a message back from the CAN bus


        nbytes = read( s, &frame, sizeof(struct can_frame));
        printf("Identifiant: %x [%d] ", frame.can_id, frame.can_dlc);
        for(i=0; i<frame.can_dlc; i++)
        printf("%x ",frame.data[i]);
        printf("\n");

        if(frame.can_id == 0x11)
        {
            printf("je detecte bien le message de l identifiant\n");
            printf("Identifiant: %x [%d] ", frame.can_id, frame.can_dlc);
            for(i=0; i<frame.can_dlc; i++)
	    {
            printf("%x ",frame.data[i]);
	    c=frame.data[i];
	    buffer[i]=c;
	    }

	    printf("%x\n", frame.data);

        }

    }
*/
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
