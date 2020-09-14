/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;
void welcome(int sd);
void login(int sd);
void parseaza(int sd,int cod);
void getUser(int sd);

void parseaza(int sd,int cod){
    if(cod == 1) getUser(sd);
    else if(cod == 2) login(sd);
    else printf("Optiunea nu exista...\n");
}

void getUser(int sd){
  char name[30],password[30];
  bzero(name,30);
  if (read (sd, &name, 30) <= 0)
  {
    perror ("Eroare la read() de la server.\n");
    return;
  }
  bzero(password,30);
  if(read(sd, &password, 30) <= 0)
  {
    perror ("Eroare la read() de la server.\n");
    return;
  }

  printf("username: %s\nparola: %s\n",name,password);
}

void login(int sd){
    char username[30],parola[30],rasp[10];

    printf("id: ");
    scanf("%s",username);
    printf("parola: ");
    scanf("%s",parola);

    if(write(sd,username,30)<=0){
        perror ("[server]Eroare la write() catre server.\n");
    }
    if(write(sd,parola,30)<=0){
        perror ("[server]Eroare la write() catre server.\n");
    }
    printf("Am trimis mesajele.Asteptam raspuns..\n");

    if(read(sd,rasp,10)<=0){
        perror ("[server]Eroare la read() de la server.\n");
    }

    if(strcmp("ok",rasp) == 0){
      printf("Te-ai logat. Yey\n");
      //userMenu(sd);
    }
    else if(strcmp("not ok",rasp) == 0)
      printf("Nu te-ai logat..Pff\n");
    else
      printf("Eroare cv idk..%s\n",rasp);   

}

void welcome(int sd){
    int nr=0;
    char buf[10];
    while(1){
        /* citirea mesajului */
        //printf("\nOptiuni:\n1.inregistrare\n2.login\n3.trimite mesaj\n0.exit\n");
        printf("\nOptiuni:\n1.getUser\n2.login\n0.exit\n");
        printf ("[client]Introduceti un numar: ");
        fflush (stdout);
        read (0, buf, sizeof(buf));
        nr=atoi(buf);
        //scanf("%d",&nr);
  
        /* trimiterea mesajului la server */
        if (write (sd,&nr,sizeof(int)) <= 0)
            {
            perror ("[client]Eroare la write() spre server.\n");
            return;
            }

        if(nr==0){printf("Bye\n");break;}
        else parseaza(sd,nr);
    }

}

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  		// mesajul trimis
  int nr=0;
  char buf[10];

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

    //incepem comunicarea cu serverul
  welcome(sd);

  /* inchidem conexiunea, am terminat */
  close (sd);
}