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
int welcome(int sd);
struct user
{
  short logged;
  char username[20];
  char type[10]; //?not sure
};
struct user user; //userul curent

void topmusic(int sd)
{
  printf("SALUUT,Te-ai logat!\n ");
}

void login(int sd)
{
  printf("sunt in login\n");
  char username[20] = "", parola[20] = "", response[2] = "";
  bzero(username, 20);
  bzero(parola, 20);
  printf("id: ");
  scanf("%s", username);
  printf("parola: ");
  scanf("%s", parola);

  if (write(sd, &username, 20) <= 0)
  {
    perror("[Client] Eroare la write catre server.\n");
    close(sd);
  }
  //printf("1");
  bzero(response, 2);
  //printf("2");
  //fflush(stdout);
  //printf("3");
  if (read(sd, &response, sizeof(response)) <= 0)
    perror("[Client] Eroare la citirea raspunsului de la server.");
  else
  {
    printf("%s\n", response);
    if (strcmp(response, "ok") != 0)
      perror("[Client] Eroare (?)\n");
  }

  if (write(sd, &parola, 20) <= 0)
  {
    perror("[Client] Eroare la write catre server.\n");
    close(sd);
  }
  else
  {
    char a[2] = "";
    bzero(a, 2);
    fflush(stdout);
    if (read(sd, &a, 2) <= 0) //citim raspunsul "corect"/"incorect" de la server
    {
      perror("[Client] Eroare la read() de la server.\n");
      close(sd);
    }
    else
    {
      printf("[login] am citit de la server: %s\n",a);
      if (strcmp(a, "ok")==0)
      {
        user.logged = 1;
        strcpy(user.username, username);
        topmusic(sd);
      }
      else
      {
        printf("[client] Eroare la login!\n");
        close(sd);
      }
    }
  }
}

void inregistrare(int sd)
{
  char username[20] = "u", password[20] = "", response[2] = "";
  printf("\n INREGISTRARE \n Introduceti datele (max 20 caractere) si apasati enter.\n");
  printf("Scrieti comanda 'back' pentru a merge pe prima pagina\n");

  while(1){
  printf("username: ");
  bzero(username, 20);
  scanf("%s", username);

  //trimitem username
  if (write(sd, &username, 20) <= 0)
  { 
    perror("[Client] Eroare la write catre server.\n");
    close(sd);
  }
  //BACK
  if(strcmp(username,"back")==0){
    printf("Redirectionare catre prima pagina..\n");
    welcome(sd);
  }
  //printf("1");
  //pregatim pt citirea raspunsului de la server
  bzero(response, 2);
  //printf("2");
  //printf("3");
  if (read(sd, &response, sizeof(response)) <= 0)
    perror("[Client] Eroare la citirea raspunsului de la server.\n");
  else
  {
    printf("%s\n", response);
    if (strcmp(response, "ok") != 0)
      printf("[Client] Username-ul exista in baza de date.\n");
    else break;
  }
  fflush(stdout);
  }
  printf("parola: ");
  scanf("%s", password);
  //BACK
  if(strcmp(password,"")==0){
    printf("Parola trebuie completata. Redirectionare catre prima pagina..\n");
    welcome(sd);
  }
  //trimitem parola
  if (write(sd, &password, 20) <= 0)
  {
    perror("[Client] Eroare la write catre server.\n");
    close(sd);
  }
  else
  {
    //citim rapunsul de la server->sunt datele valide si disponibile?
    char a[10] = "";
    bzero(a, 10);
    fflush(stdout);
    if (read(sd, &a, 10) <= 0) //citim raspunsul "corect"/"incorect" de la server
    {
      perror("[Client] Eroare la read() de la server.\n");
      close(sd);
    }
    else
    {
      if (strcmp(a, "corect"))
      {
        printf("Inregistrare completa! Redirectionare catre prima pagina..\n");
      }
      else
      {
        printf("[client] Eroare la inregistrare! \n");
        welcome(sd); //redirectionare
      }
    }
  }
}
int welcome(int sd)
{
  printf("\t Bine ai venit!");
  printf("Scrie-mi ce doresti sa faci mai departe\n->login\n->inregistrare\n->exit\n");
  char command[30];
  int a;
  scanf("%s", command);
  if ((a=write(sd, &command, sizeof(command))) <= 0)
  {
    printf("Eroare la scriere catre server.\n");
    return errno;
  }
  else
  {
    printf("[welcome]am citit: %d biti\n",a);
    a=0;
    fflush(stdout);
    if (read(sd, &a, sizeof(a)) <= 0)
    {
      printf("Eroare la citirea de la server.\n");
      return errno;
    }
    else
    {
      printf("[welcome]a=%d\n",a);
      switch (a)
      {
      case 1:
        a = 1;
        login(sd);
        break;
      case 2:
        a = 2;
        inregistrare(sd);
        break;
      case 0:
        a = 0;
        printf("[welcome]See ya!\n");
        close(sd);
        break;
      default:
        printf("[welcome]Eroare server.\n");
        close(sd);
        a=-1;
        break;
      }
    }
  }
  return a;
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
  
  /* inchidem conexiunea, am terminat */
  int a = 1;
  while(a > 0){
    a=welcome(sd);
    printf("a=%d",a);
  }
  printf("MAIN:am iesit");
  close(sd);
}
