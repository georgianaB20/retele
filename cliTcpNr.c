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
#include <arpa/inet.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct user
{
  char name[30];
  char password[30];
  int vote;
  int admin;
} user;

/* portul de conectare la server*/
int port;
struct user *usr;
void welcome(int sd);
void login(int sd);
void parseaza(int sd, int cod,char from[10]);
void getUser(int sd);
void inregistrare(int sd);
void adaugaAdmin(int sd);

void parseaza(int sd, int cod,char from[10])
{
  if(strcmp(from,"userMenu")==0)
  //tratam comenzile din userMenu
  {
    if(usr->admin==1){
      if (cod == 1) getUser(sd);
      else if (cod == 2) adaugaAdmin(sd);
      else printf("Optiunea nu exista...\n");
    }
    else
    {
      if (cod == 1) getUser(sd);
      else printf("Optiunea nu exista...\n");
    }
    
  }
  else if(strcmp(from,"welcome")==0)
  //tratam comenzile din welcome
  {
    if(cod == 1) login(sd);
    else if (cod == 2) inregistrare(sd);
    else printf("Optiunea nu exista...\n");    
  }
}

void getUser(int sd)
{
  //printf("date: %s %s\n",usr->name,usr->password);
  char name[30], password[30];
  int vote,admin;
  bzero(name, 30);
  if (read(sd, &name, 30) <= 0)
  {
    perror("Eroare la read() de la server.\n");
    return;
  }
  bzero(password, 30);
  if (read(sd, &password, 30) <= 0)
  {
    perror("Eroare la read() de la server.\n");
    return;
  }
  if (read(sd, &vote, sizeof(int)) <= 0)
  {
    perror("Eroare la read() de la server.\n");
    return;
  }
  if (read(sd, &admin, sizeof(int)) <= 0)
  {
    perror("Eroare la read() de la server.\n");
    return;
  }
  if (strlen(name) == 0 && strlen(password) == 0)
    printf("Nu sunteti logat.\n");
  else
    printf("\nusername: %s\nparola: %s\nvote: %s\nadmin: %s\n", name, password,vote==1? "yes":"no",admin==1?"yes":"no");
}

void inregistrare(int sd)
{
  char username[30], parola[30], rasp[50];

  printf("id: ");
  scanf("%s", username);
  printf("parola: ");
  scanf("%s", parola);

  if (write(sd, username, 30) <= 0)
  {
    perror("[server]Eroare la write() catre server.\n");
  }
  if (write(sd, parola, 30) <= 0)
  {
    perror("[server]Eroare la write() catre server.\n");
  }
  printf("Am trimis mesajele.Asteptam raspuns..\n");

  if (read(sd, rasp, 50) <= 0)
  {
    perror("[server]Eroare la read() de la server.\n");
  }
  else
    printf("%s", rasp);
}

void adaugaAdmin(int sd)
{
  char username[30], rasp[50];

  printf("Introduceti username-ul utilizatorului pe care doriti sa il promovati admin:\n ");
  scanf("%s", username);

  if (write(sd, username, 30) <= 0)
  {
    perror("[server]Eroare la write() catre server.\n");
  }
  printf("Am trimis mesajele.Asteptam raspuns..\n");

  if (read(sd, rasp, 50) <= 0)
  {
    perror("[server]Eroare la read() de la server.\n");
  }
  else
    printf("%s", rasp);
}

void userMenu(int sd){
  int nr = 0;
  char buf[10];
  while (1)
  {
    //printf("%d,%d\n",usr->admin,usr->admin==1? 1:0);
    /* citirea mesajului */
    if(usr->admin==1)
      printf("\nOptiuni:\n1.Cont\n2.Adauga admin\n0.logout\n");
    else
      printf("\nOptiuni:\n1.Cont\n0.logout\n");
    
    printf("[client]Introduceti un numar: ");
    fflush(stdout);
    read(0, buf, sizeof(buf));
    nr = atoi(buf);
    //scanf("%d",&nr);

    /* trimiterea mesajului la server */
    if (write(sd, &nr, sizeof(int)) <= 0)
    {
      perror("[client]Eroare la write() spre server.\n");
      return;
    }

    if (nr == 0)
    {
      strcpy(usr->name,"");
      strcpy(usr->password,"");
      usr->admin=0; usr->vote=0;
      printf("Te-ai delogat din cont.\n");
      break;
    }
    else
      parseaza(sd, nr,"userMenu");
  }
}

void login(int sd)
{
  //printf("%ld %ld", strlen(usr->name), strlen(usr->password));
  if (strlen(usr->name) > 0 && strlen(usr->password) > 0)
  {
    printf("Sunteti deja logat ca : %s", usr->name);
    return;
  }
  else
  {
    char username[30], parola[30], rasp[50];

    printf("id: ");
    scanf("%s", username);
    printf("parola: ");
    scanf("%s", parola);

    if (write(sd, username, 30) <= 0)
    {
      perror("[server]Eroare la write() catre server.\n");
      return;
    }
    if (write(sd, parola, 30) <= 0)
    {
      perror("[server]Eroare la write() catre server.\n");
      return;
    }
    printf("Am trimis mesajele.Asteptam raspuns..\n");
    if (read(sd, rasp, 50) <= 0)
    {
      perror("[server]Eroare la read() de la server.\n");
      return;
    }
    printf("%s",rasp);
    fflush(stdout);
    if (strcmp("Salut! Te-ai logat cu succes.\n", rasp) == 0)
    {
      strcpy(usr->name, username);
      strcpy(usr->password, parola);
      //cerem celelalte date ale userului
      if(read(sd,&usr->admin,sizeof(int)) <= 0)
      {
      perror("[server]Eroare la read() de la server.\n");
      return;
      }

      if(read(sd,&usr->vote,sizeof(int)) <= 0)
      {
      perror("[server]Eroare la read() de la server.\n");
      return;
      }

      userMenu(sd);
    }
  } //else
}

void welcome(int sd)
{
  int nr = 0;
  char buf[10];
  while (1)
  {
    /* citirea mesajului */
    //printf("\nOptiuni:\n1.inregistrare\n2.login\n3.trimite mesaj\n0.exit\n");
    printf("\nOptiuni:\n1.login\n2.inregistrare\n0.exit\n");
    printf("[client]Introduceti un numar: ");
    fflush(stdout);
    read(0, buf, sizeof(buf));
    nr = atoi(buf);
    //scanf("%d",&nr);

    /* trimiterea mesajului la server */
    if (write(sd, &nr, sizeof(int)) <= 0)
    {
      perror("[client]Eroare la write() spre server.\n");
      return;
    }

    if (nr == 0)
    {
      printf("Bye\n");
      break;
    }
    else
      parseaza(sd, nr, "welcome");
  }
}

int main(int argc, char *argv[])
{
  int sd;                    // descriptorul de socket
  struct sockaddr_in server; // structura folosita pentru conectare
      // mesajul trimis
  int nr = 0;
  char buf[10];

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
  {
    printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }

  /* stabilim portul */
  port = atoi(argv[2]);

  /* cream socketul */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Eroare la socket().\n");
    return errno;
  }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons(port);

  /* ne conectam la server */
  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la connect().\n");
    return errno;
  }

  //incepem comunicarea cu serverul
  usr = (struct user *)malloc(sizeof(struct user));
  welcome(sd);

  /* inchidem conexiunea, am terminat */
  close(sd);
}