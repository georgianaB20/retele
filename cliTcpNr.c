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

typedef struct song
{
  int id;
  char name[50];
  char artist[50];
  int votes;
  char link[100];
  char description[100];
  char genres[100];
} song;

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
void parseaza(int sd, int cod, char from[10]);
void getUser(int sd);
void inregistrare(int sd);
void adaugaAdmin(int sd);
void banUserVote(int sd);
void addSong(int sd);
void getTop(int sd);
void cautaCantec(int sd);
void deleteSong(int sd);

void parseaza(int sd, int cod, char from[10])
{
  if (strcmp(from, "userMenu") == 0)
  //tratam comenzile din userMenu
  {
    if (usr->admin == 1)
    {
      if (cod == 1)
        getUser(sd);
      else if (cod == 2)
        adaugaAdmin(sd);
      else if (cod == 3)
        banUserVote(sd);
      else if (cod == 4)
        addSong(sd);
      else if (cod == 5)
        getTop(sd);
      else if (cod == 6)
        cautaCantec(sd);
      else if (cod == 7)
        deleteSong(sd);
      else
        printf("Optiunea nu exista...\n");
    }
    else
    {
      if (cod == 1)
        getUser(sd);
      else if (cod == 2)
        addSong(sd);
      else if (cod == 3)
        getTop(sd);
      else if (cod == 4)
        cautaCantec(sd);
      else
        printf("Optiunea nu exista...\n");
    }
  }
  else if (strcmp(from, "welcome") == 0)
  //tratam comenzile din welcome
  {
    if (cod == 1)
      login(sd);
    else if (cod == 2)
      inregistrare(sd);
    else
      printf("Optiunea nu exista...\n");
  }
}

void getUser(int sd)
{
  //printf("date: %s %s\n",usr->name,usr->password);
  char name[30], password[30];
  int vote, admin;
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
  printf("----------------------------------------------\n");
  if (strlen(name) == 0 && strlen(password) == 0)
    printf("Nu sunteti logat.\n");
  else
    printf("\nusername: %s\nparola: %s\nvote: %s\nadmin: %s\n", name, password, vote == 1 ? "yes" : "no", admin == 1 ? "yes" : "no");
    
}

void getTop(int sd)
{
  printf("Introduceti \"general\" sau numele unui gen muzical pentru a filtra topul.\n");

  char type[15];
  bzero(type, 15);
  read(0, type, 15);
  //printf("%s",type);

  if (write(sd, type, 15) <= 0)
  {
    perror("[client]Eroare la write() spre client.\n");
    return;
  }

  int lungime;
  if (read(sd, &lungime, sizeof(int)) <= 0)
  {
    perror("[server]Eroare la read() de la client.\n");
    return;
  }

  char *top = malloc(lungime + 1);
  if (read(sd, top, lungime) <= 0)
  {
    perror("[server]Eroare la read() de la client.\n");
    return;
  }
  printf("----------------------------------------------\n");
  printf("%s", top);
}

void inregistrare(int sd)
{
  char username[30], parola[30], rasp[50];
  printf("----------------------------------------------\n");
  printf("id: ");
  scanf("%s", username);
  printf("parola: ");
  scanf("%s", parola);
  printf("----------------------------------------------\n");

  if (write(sd, username, 30) <= 0)
  {
    perror("[server]Eroare la write() catre server.\n");
  }
  if (write(sd, parola, 30) <= 0)
  {
    perror("[server]Eroare la write() catre server.\n");
  }
  //printf("Am trimis mesajele.Asteptam raspuns..\n");

  if (read(sd, rasp, 50) <= 0)
  {
    perror("[server]Eroare la read() de la server.\n");
  }
  else
    printf("%s", rasp);
  printf("----------------------------------------------");
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

void banUserVote(int sd)
{
  char username[30], rasp[50];
  printf("------------------------------------------------\n");
  printf("Username-ul utilizatorului ce urmeaza a fi restrictionat:\n ");
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
  printf("------------------------------------------------\n");
    printf("%s", rasp);
  printf("------------------------------------------------\n");
}

void addSong(int sd)
{
  char nume[50], artist[50], link[100], description[100], genres[100];
  //char *nume,*artist,*link,*description;
  //citim datele de la user
  printf("------------------------------------------------\n");
  bzero(nume, 50);
  printf("Introduceti datele melodiei:\nTitlu: ");
  fflush(stdout);
  read(0, nume, 50);
  bzero(artist, 50);
  printf("Artist: ");
  fflush(stdout);
  read(0, artist, 50);
  bzero(link, 100);
  printf("Link: ");
  fflush(stdout);
  read(0, link, 100);
  bzero(description, 100);
  printf("Descriere: ");
  fflush(stdout);
  read(0, description, 100);
  printf("Gen muzical: ");
  fflush(stdout);
  bzero(genres, 100);
  read(0, genres, 100);

  if (nume[strlen(nume) - 1] == '\n')
    nume[strlen(nume) - 1] = '\0';

  if (artist[strlen(artist) - 1] == '\n')
    artist[strlen(artist) - 1] = '\0';

  if (link[strlen(link) - 1] == '\n')
    link[strlen(link) - 1] = '\0';

  if (description[strlen(description) - 1] == '\n')
    description[strlen(description) - 1] = '\0';

  if (genres[strlen(genres) - 1] == '\n')
    genres[strlen(genres) - 1] = '\0';

  //printf("%s %s %s %s    /%s/\n", nume, artist, link, description, genres);
  int lungime = strlen(nume);
  //trimitem datele la server
  write(sd, &lungime, sizeof(int));
  if (write(sd, nume, lungime) <= 0)
  {
    perror("[server]Eroare la write() catre server.\n");
    return;
  }
  lungime = strlen(artist);
  write(sd, &lungime, sizeof(int));
  if (write(sd, artist, lungime) <= 0)
  {
    perror("[server]Eroare la write() catre server.\n");
    return;
  }
  lungime = strlen(description);
  write(sd, &lungime, sizeof(int));
  if (write(sd, description, lungime) <= 0)
  {
    perror("[server]Eroare la write() catre server.\n");
    return;
  }
  lungime = strlen(link);
  //printf("lungime link=%d\n",lungime);
  write(sd, &lungime, sizeof(int));
  if (write(sd, link, lungime) <= 0)
  {
    perror("[server]Eroare la write() catre server.\n");
    return;
  }

  lungime = strlen(genres);
  //printf("lungime link=%d\n",lungime);
  write(sd, &lungime, sizeof(int));
  if (write(sd, genres, lungime) <= 0)
  {
    perror("[server]Eroare la write() catre server.\n");
    return;
  }

  //prelucrare string de genuri

  char rasp[50];
  printf("------------------------------------------------\n");
  //citeste raspuns de la server
  if (read(sd, rasp, 50) <= 0)
  {
    perror("[server]Eroare la read() de la server.\n");
  }
  else
    printf("%s", rasp);
  
}

void deleteSong(int sd)
{
  char nume[50], artist[50];
  printf("------------------------------------------------\n");

  bzero(nume, 50);
  printf("Introduceti datele melodiei:\nTitlu: ");
  fflush(stdout);
  read(0, nume, 50);
  bzero(artist, 50);
  printf("Artist: ");
  fflush(stdout);
  read(0, artist, 50);

  if (nume[strlen(nume) - 1] == '\n')
    nume[strlen(nume) - 1] = '\0';

  if (artist[strlen(artist) - 1] == '\n')
    artist[strlen(artist) - 1] = '\0';

  if (write(sd, nume, 50) <= 0)
  {
    perror("[server]Eroare la write() catre server.\n");
    return;
  }

  if (write(sd, artist, 50) <= 0)
  {
    perror("[server]Eroare la write() catre server.\n");
    return;
  }

  printf("------------------------------------------------\n");
  char rasp[50];
  bzero(rasp, 50);
  if (read(sd, rasp, 50) <= 0)
  {
    perror("[server]Eroare la write() catre server.\n");
    return;
  }
  printf("%s", rasp);
  printf("------------------------------------------------\n");
}

void cautaCantec(int sd)
{
  char nume[50], artist[50];

  printf("------------------------------------------------\n");
  bzero(nume, 50);
  printf("Introduceti datele melodiei:\nTitlu: ");
  fflush(stdout);
  read(0, nume, 50);
  bzero(artist, 50);
  printf("Artist: ");
  fflush(stdout);
  read(0, artist, 50);

  if (nume[strlen(nume) - 1] == '\n')
    nume[strlen(nume) - 1] = '\0';

  if (artist[strlen(artist) - 1] == '\n')
    artist[strlen(artist) - 1] = '\0';

  if (write(sd, nume, 50) <= 0)
  {
    perror("[server]Eroare la write() catre server.\n");
    return;
  }

  if (write(sd, artist, 50) <= 0)
  {
    perror("[server]Eroare la write() catre server.\n");
    return;
  }
  int check;
  if (read(sd, &check, sizeof(int)) <= 0)
  {
    perror("[server]Eroare la read() de la server.\n");
  }
  //verificam daca cantecul exista
  if (check != 0)
  {
    song *s = (struct song *)malloc(sizeof(struct song));
    long unsigned int lungime;
    //read(sd,&lungime,sizeof(long unsigned int));
    if (read(sd, s->artist, 50) <= 0)
    {
      perror("[server]Eroare la read() de la server.\n");
    }

    if (read(sd, s->name, 50) <= 0)
    {
      perror("[server]Eroare la read() de la server.\n");
    }

    if (read(sd, s->description, 100) <= 0)
    {
      perror("[server]Eroare la read() de la server.\n");
    }

    if (read(sd, s->link, 100) <= 0)
    {
      perror("[server]Eroare la read() de la server.\n");
    }

    if (read(sd, &s->id, sizeof(int)) <= 0)
    {
      perror("[server]Eroare la read() de la server.\n");
    }

    char *comentarii;
    read(sd, &lungime, sizeof(long unsigned int));
    comentarii = malloc(lungime + 1);
    bzero(comentarii, lungime);
    if (read(sd, comentarii, lungime) <= 0)
    {
      perror("[server]Eroare la read() de la server.\n");
    }

    char *rating;
    read(sd, &lungime, sizeof(long unsigned int));
    rating = malloc(lungime + 1);
    bzero(rating, lungime);
    if (read(sd, rating, lungime) <= 0)
    {
      perror("[server]Eroare la read() de la server.\n");
    }

    printf("------------------------------------------------");
    printf("\nArtist: %s\n", s->artist);
    printf("Titlu: %s\n", s->name);
    printf("Link: %s\n", s->link);
    printf("Descriere: %s\n", s->description);
    //printf("Gen muzical: %s",)
    printf("Rating: %s\n", rating);
    printf("%s", comentarii);
    //printf("------------------------------------------------\n");

    char buf[10];
    int optiune;
    while (1)
    {
      printf("------------------------------------------------\n");
      printf("\n1.Voteaza cantec\n2.Comenteaza\n0.Inapoi la meniul principal\nOptiunea ta:");
      //scanf("%d",optiune);
      fflush(stdout);
      read(0, buf, sizeof(buf));
      optiune = atoi(buf);

      if (write(sd, &optiune, sizeof(int)) <= 0)
      {
        perror("[server]Eroare la write() catre server.\n");
        return;
      }

      if (optiune == 1)
      {
        printf("Votati cu un numar intre 1 si 5:");
        fflush(stdout);
        float vot;
        //scanf("%2f",vot);
        bzero(buf, 10);
        read(0, buf, sizeof(buf));
        vot = atof(buf);

        if (write(sd, &vot, sizeof(float)) <= 0)
        {
          perror("[server]Eroare la write() catre server.\n");
          return;
        }

        char rasp[50];
        if (read(sd, rasp, 50) <= 0)
        {
          perror("[server]Eroare la read() de la server.\n");
          return;
        }
        printf("%s", rasp);
      }
      else if (optiune == 2)
      {
        printf("Comentariul tau:\n");
        fflush(stdout);
        char comment[100];
        bzero(comment, 100);
        read(0, comment, 100);
        //adaugaComentariu(sd,comment,s->id);
        if (write(sd, comment, 100) <= 0)
        {
          perror("[server]Eroare la write() catre server.\n");
          return;
        }

        char rasp[50];
        if (read(sd, rasp, 50) <= 0)
        {
          perror("[server]Eroare la read() de la server.\n");
          return;
        }
        printf("%s", rasp);
      }
      else if (optiune == 0)
        break;
      else
        printf("Optiunea nu exista.\n");
    }
  }
  else
  {
    printf("Melodia nu exista in baza de date\n");
  }
}

void userMenu(int sd)
{
  int nr;
  char buf[10];
  while (1)
  {

    /* citirea mesajului */
    printf("------------------------------------------------\n");
    if (usr->admin == 1)
      printf("\nOptiuni:\n1.Cont\n2.Adauga admin\n3.Restrictionare vot\n4.Adauga melodie\n5.Topuri\n6.Cauta cantec\n7.Sterge melodie\n0.logout\n");
    else
      printf("\nOptiuni:\n1.Cont\n2.Adauga melodie\n3.Topuri\n4.Cauta cantec\n0.logout\n");

    printf("Introduceti un numar: ");
    fflush(stdout);
    read(0, buf, sizeof(buf));
    nr = atoi(buf);

    /* trimiterea mesajului la server */
    if (write(sd, &nr, sizeof(int)) <= 0)
    {
      perror("[client]Eroare la write() spre server.\n");
      return;
    }

    if (nr == 0)
    {
      bzero(usr, sizeof(usr));
      printf("------------------------------------------------\n");
      printf("BYE\n");
      break;
    }
    else
      parseaza(sd, nr, "userMenu");
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
    printf("%s", rasp);
    fflush(stdout);
    if (strcmp("Salut! Te-ai logat cu succes.\n", rasp) == 0)
    {
      strcpy(usr->name, username);
      strcpy(usr->password, parola);
      //cerem celelalte date ale userului
      if (read(sd, &usr->admin, sizeof(int)) <= 0)
      {
        perror("[server]Eroare la read() de la server.\n");
        return;
      }

      if (read(sd, &usr->vote, sizeof(int)) <= 0)
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
  int nr;
  char buf[10];
  while (1)
  {
    /* citirea mesajului */
    printf("------------------------------------------------");
    printf("\nOptiuni:\n1.login\n2.inregistrare\n0.exit\n");
    printf("Introduceti un numar: ");
    fflush(stdout);
    read(0, buf, sizeof(buf));
    nr = atoi(buf);

    /* trimiterea mesajului la server */
    if (write(sd, &nr, sizeof(int)) <= 0)
    {
      perror("[client]Eroare la write() spre server.\n");
      return;
    }

    if (nr == 0)
    {
      printf("------------------------------------------------\n");
      printf("BYE\n");
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