#include "echange_tcp.h"

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

        if( data == ' ' )
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

int saveTrame(FILE* fptr, char *buffer, int i)
{



        //for(i=0 ; i<TAILLE_TRAME ; i++)
        //{
        fprintf(fptr, "%d,%s\n", i, buffer);
        //}

 return 1;
}
