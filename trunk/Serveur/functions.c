#include "functions.h"

ssize_t ecrireLigne(int sock, const void *buffer, size_t nb){
    ssize_t aEcrire, ecrits;
    aEcrire = nb;
    while( aEcrire > 0){
        ecrits = write(sock, buffer, aEcrire);
        if( ecrits <= 0){
            return ecrits;  /*erreur ou fin d'ecriture*/
    }
        aEcrire -= ecrits;  /*on decompte le nombre d'octets deja ecrits*/
        buffer += ecrits;   /*on se deplace du nombre d'octets deja ecrits*/
    }
    ecrits = write(sock, "\r\n", 2);
    if( ecrits == 2){
        return 1;   //ok
    }
    return ecrits;
}

ssize_t lireLigne(int sock, void *buffer, size_t nbMax){
    ssize_t aLire, lus;
    char data;
    char *debut = (char *)buffer;
    char *ligne;

    aLire = nbMax;
    while( aLire > 0 ){
        lus = read(sock, &data, 1);
        if( lus < 0){
            return lus;     /*erreur*/
        }else if( lus == 0 ){
                return 0;   /*fin de lecture*/
        }
        *(char *)buffer = data;
        aLire -= lus;   /*on decompte le nombre d'octets deja lus*/
        buffer +=lus;   /*on se deplace du nombre d'octets deja lus*/
        *(char *)buffer = 0x00; /*fin de chaine*/
         /*on recherche les delimiteurs "\r\n"*/
        ligne = (char *)strstr(debut, "\r\n");
        if(ligne != NULL){
            buffer -= 2;    /*on se recule du nombre d'octets des delimiteurs*/
            *(char *)buffer = 0x00; /*fin de chaine*/
            return 1;   //ok
        }
    }
    return (nbMax - aLire);
}

ssize_t lectureTrame(int liaisonSerie, char *buffer, size_t tailleBuffer)
{
    ssize_t lus, totalLus = 0;
    char data;
    int i = 0;
    int lecture = 0;

    while( i < tailleBuffer )
    {
        printf("je rentre dans la liaison\n");
        lus = read(liaisonSerie, &data, sizeof data);
        if( lus < 0)
        {
            printf("erreur : lus = %d \n", lus);
            return lus;     /*erreur*/
        }


        totalLus += lus;
        printf("%c\n", data);

        if(lecture)
        {
            printf("ajout d'une data au buffer : %c\n", data);
            buffer[i] = data;
            i++;
        }

        if( data == '+' )
        {
            printf("caractere de debut de trame detecte\n");
            if(lecture == 0)
            {
                lecture = 1;
                printf("mode lecture active\n");
            }
           // if(lecture == 1) return;
        }
    }

    return totalLus;
}

ssize_t lectureTrameCan(char *buffer, size_t tailleBuffer)
{
        printf("je suis dans le thread tramecan\n");

        int s,i;
        int nbytes;
        int save;
        int j=0;
        struct sockaddr_can addr;
        struct can_frame frame;
        struct ifreq ifr;
        char c;
        //char bufferCan[TAILLE_TRAME_CAN];
        char *ifname = "can0";
		int totalLus = 0;

		// INIT CAN
			FILE* fptr = fopen("dataCAN.csv", "w");

			if((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
			{
					perror("Error while opening socket");
					return -1;
			}

			printf("socket canbus cree avec sucees\n");

			strcpy(ifr.ifr_name, ifname);
			ioctl(s, SIOCGIFINDEX, &ifr);

			addr.can_family  = AF_CAN;
			addr.can_ifindex = ifr.ifr_ifindex;

			printf("%s at index %d\n", ifname, ifr.ifr_ifindex);

			if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
			{
					perror("Error in socket bind");
					return -2;
			}
			printf("socket attache avec succes\n");
		// INIT CAN

        //frame.can_id  = 0x123;
        //frame.can_dlc = 2;
        //frame.data[0] = 0xB0;
        //frame.data[1] = 0x0B;

        //nbytes = write(s, &frame, sizeof(struct can_frame));
        //printf("Wrote %d bytes\n", nbytes);

        //frame.can_id  = 0x456;
        //frame.can_dlc = 2;
        //frame.data[0] = 0x13;
        //frame.data[1] = 0x37;

        //nbytes = write(s, &frame, sizeof(struct can_frame));
        //printf("Wrote %d bytes\n", nbytes);

        while(keepRunning)
        {
			//Read a message back from the CAN bus

            nbytes = read( s, &frame, sizeof(struct can_frame));

            printf("Identifiant: %x [%d] ", frame.can_id, frame.can_dlc);
            for(i=0; i<frame.can_dlc; i++)
			{
				printf("%x ",frame.data[i]);
			}
			printf("\n");

            if(frame.can_id == 0x11)
            {
                printf("je detecte bien le message de l identifiant 11\n");
                printf("Identifiant: %x [%d]\n", frame.can_id, frame.can_dlc);
                for(i=0; i<frame.can_dlc; i++)
                {
                    //printf("%d\n",frame.data[i]);
                    buffer[i] = frame.data[i];
                    printf("%d\n", buffer[i]);
					totalLus++;
                }

                save = saveTrameCan(fptr, bufferCan, j, TAILLE_TRAME_CAN);
                j++;

				return totalLus;
            }
        }

        return 0;
}

int saveTrame(FILE* fptr, char *buffer, int j, int sizeofbuffer)
{
    int i;
    for(i=0 ; i<sizeofbuffer ; i++)
    {
        if(buffer[i] == ' ')
        {
            printf("je detecte bien lespace\n");
            buffer[i]=';';
        }
    }
    printf("%s\n", buffer);
    fprintf(fptr, "%d;%s\n", j, buffer);


 return 1;
}

int saveTrameCan(FILE* fptr, char *bufferCan, int j, int sizeofbuffercan, int test = 0)
{
	//static int ligne = 0;

	printf("Je suis dans la fonction save tramecan\n");
	printf("%d, %d, %d, %d, %d, %d, %d, %d\n", *bufferCan, *(bufferCan+1), *(bufferCan+2), *(bufferCan+3), *(bufferCan+4), *(bufferCan+5), *(bufferCan+6), *(bufferCan+7));
	fprintf(fptr, "%d;%d;%d;%d;%d;%d;%d;%d;%d\n", j, *bufferCan, *(bufferCan+1), *(bufferCan+2), *(bufferCan+3), *(bufferCan+4), *(bufferCan+5), *(bufferCan+6), *(bufferCan+7));

	//ligne++;
}

int numberOfEncodingDigits(int number)
{
	if( number < 0 ) return 0;
	if( number < 10 ) return 1;
	if( number < 100) return 2;
	if( number < 1000) return 3;
	else return 0;
}

void convertIntToChar(int value, char* result, int resultSize)
{
	char* buffer;

	digits = numberOfEncodingDigits(value);
	if( digits == 0 )
	{
		// error
		exit 0;
	}
	buffer = (char*)malloc(digits+1); // +1 for \n
	sprintf(buffer, "%d", value);

	j=0;
	for(i = 0 ; i < resultSize ; i++)
	{
		if( i < resultSize-digits ) result[i] = '0';
		else
		{
			result[i] = buffer[j];
			j++;
		}
	}

	free(buffer);
}

int concatenation(char* frameSerie, char* frameCan, char* tailleTrameSerieLue_encode, char* tailleTrameCanLue_encode)
{

int i, j;
sizeof longueurTrame;
longueurTrame = 30;

char* trameTotal = malloc(longueurTrame);


    for(i=0, j=0 ; i<longueurTrame ; i++ , j++)
    {
        if (i<TAILLE_INFO_TRAME+TAILLE_INFO_TRAME_CAN) trameTotal[i] = tailleTrameSerieLue_encode[j];
        if (i==TAILLE_INFO_TRAME+TAILLE_INFO_TRAME_CAN) j=0;
        if (i<TAILLE_INFO_TRAME) trameTotal[i] = tailleTrameCanLue_encode[j];
        if (i==TAILLE_INFO_TRAME) j=0;
        if (i<TAILLE_INFO_TRAME+TAILLE_INFO_TRAME_CAN+TAILLE_TRAME) trameTotal[i] = frameSerie[j];
        if (i==TAILLE_INFO_TRAME+TAILLE_INFO_TRAME_CAN+TAILLE_TRAME) j=0;
        if (i<TAILLE_INFO_TRAME+TAILLE_INFO_TRAME_CAN+TAILLE_TRAME+TAILLE_TRAME_CAN) trameTotal[i] = frameCan[j];
    }

}
