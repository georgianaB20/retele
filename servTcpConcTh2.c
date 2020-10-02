/* servTCPConcTh2.c - Exemplu de server TCP concurent care deserveste clientii
   prin crearea unui thread pentru fiecare client.
   Asteapta un numar de la clienti si intoarce clientilor numarul incrementat.
	Intoarce corect identificatorul din program al thread-ului.
  
   
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <mysql/mysql.h>

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData
{
  int idThread; //id-ul thread-ului tinut in evidenta de acest program
  int cl;       //descriptorul intors de accept
  struct user *client;
} thData;
//genres,votes?
typedef struct user
{
  char name[30];
  char password[30];
  int vote;
  int admin;
} user;

typedef struct comment{
  char text[250];
  char user[30];
} comment;

typedef struct song{
  int id;
  char name[50];
  char artist[50];
  int votes;
  char link[100];
  char description[100];
  
} song;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
void welcome(thData tdL);
void parseaza(thData tdL, int cod,char from[10]);
void login(thData tdL);
char* getUser(thData tdL, char username[30], char password[30]);
void sendUser(thData tdL);
void inregistrare(thData tdL);
void adaugaAdmin(thData tdL);

MYSQL *getConnection()
{
  MYSQL *conn = mysql_init(NULL);
  if (conn == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(conn));
    exit(1);
  }
  if (mysql_real_connect(conn, "localhost", "root", "elefant.2004", "proiect", 0, NULL, 0) == NULL)
  {
    fprintf(stderr, "Am dat peste o eroare: %s\n", mysql_error(conn));
    mysql_close(conn);
    exit(1);
  }
  //printf("Conectare la baza de date: [ OK ]\n");
  return conn;
}

//returneaza: 1-inserare cu succes; 0-eroare+mesaj
char *insertUser(char username[30], char password[30])
{
  char *mesaj;
  MYSQL *conn = getConnection();
  if (conn == NULL)
  {
    mesaj = "Baza de date indisponibila.Incearca din nou.\n";
    return mesaj;
  }

  char stmt[250] = "INSERT INTO Users (username,password) VALUES (\"";
  strcat(stmt, username);
  strcat(stmt, "\",\"");
  strcat(stmt, password);
  strcat(stmt, "\")");

  if (mysql_query(conn, stmt) != 0)
  {
    if (mysql_errno(conn) == 1062)
      mesaj = "Userul exista. Incearca alt username.\n";
    else
      mesaj = "Eroare la baza de date. Incearca din nou.\n";

    mysql_close(conn);
    return mesaj;
  }
  else
  {
    mysql_close(conn);
    return "Te-ai inregistrat cu succes!\n";
  }
}

char *newAdmin(char username[30]){
    char* mesaj;
    MYSQL* conn=getConnection();
    if(conn == NULL){
        return "Baza de date indisponibila.Incearca din nou.\n";
    }

    char stmt[250]="UPDATE Users SET admin=1 WHERE username like \"";
    strcat(stmt,username);
    strcat(stmt,"\"");
    //printf("%s",stmt);

    if(mysql_query(conn,stmt) != 0) {    
        if(mysql_affected_rows(conn)==0)
            mesaj="Userul nu exista.\n";
        else{
            mesaj="Eroare la baza de date.\n";
            mysql_rollback(conn);
        }
        
        mysql_close(conn);   
        return mesaj;                                                                    
    }
    else {
        mysql_close(conn);
        return "Userul a fost promovat admin!\n";
    }

}

char *getUser(thData tdL,char username[30],char password[30]){
    char* mesaj;
    MYSQL* conn=getConnection();
    if(conn == NULL){
        return "Baza de date indisponibila.Incearca din nou.\n";
    }

    char stmt[250]="SELECT * FROM Users WHERE username like \"";
    strcat(stmt,username);
    strcat(stmt,"\" and password like \"");
    strcat(stmt,password);
    strcat(stmt,"\"");

    if(mysql_query(conn,stmt) != 0){
      if(mysql_affected_rows(conn)==0)
        mesaj="User sau parola incorecte.\n";
    }
    else {
      MYSQL_RES *res=mysql_store_result(conn);
      if(res==NULL)
        mesaj="User sau parola incorecte.\n";

      int n=mysql_num_rows(res);
      if(n == 1)
      {
        MYSQL_ROW row=mysql_fetch_row(res);
        strcpy(tdL.client->name,row[0]);
        strcpy(tdL.client->password,row[1]);
        tdL.client->admin=atoi(row[2]);
        tdL.client->vote=atoi(row[3]);
        mysql_free_result(res);
        mesaj="Salut! Te-ai logat cu succes.\n";
      }
      else
      {
        mesaj="Eroare la baza de date.\n";
      }
      mysql_close(conn);
      return mesaj;
    }

}

void parseaza(thData tdL, int cod,char from[10])
{
  if(strcmp(from,"userMenu")==0)
  //tratam comenzile din userMenu
  {
    if(tdL.client->admin==1){
      if (cod == 1) sendUser(tdL);
      else if (cod == 2) adaugaAdmin(tdL);
      else return;
    }
    else
    {
      if (cod == 1) sendUser(tdL);
      else return;
    }
    
  }
  else if(strcmp(from,"welcome")==0)
  //tratam comenzile din welcome
  {
    if(cod == 1) login(tdL);
    else if (cod == 2) inregistrare(tdL);
    else return;    
  }

}

void userMenu(thData tdL){
  int nr;
  while (1)
  {
    if (read(tdL.cl, &nr, sizeof(int)) <= 0)
    {
      printf("[Thread %d]S-a intrerupt...", tdL.idThread);
      perror("Eroare la read() de la client.");
      break;
    }
    //printf ("[Thread %d]Mesajul a fost receptionat...%d\n",tdL.idThread, nr);

    if (nr == 0)
    {
      printf("[Thread %d]S-a delogat...\n", tdL.idThread);
      //free(tdL.client);
      fflush(stdout);
      break;
    }
    else
      parseaza(tdL, nr,"userMenu");
  }
}

void login(thData tdL)
{
  //verificam daca userul este logat
  if (strlen(tdL.client->name) > 0 && strlen(tdL.client->password) > 0)
  {
    return;
  }

  char username[30], parola[30];
  bzero(username, 30);
  if (read(tdL.cl, username, 30) <= 0)
  {
    perror("[server]Eroare la read() de la client.\n");
    return;
  }
  bzero(parola, 30);
  if (read(tdL.cl, parola, 30) <= 0)
  {
    perror("[server]Eroare la read() de la client.\n");
    return;
  }
  printf("[Thread %d]Login: %s. Verificam...\n", tdL.idThread, username);
  fflush(stdout);

  //VERIFICARE + RASPUNS
  char* mesaj=getUser(tdL, username, parola);
  if (write(tdL.cl, mesaj, 50) <= 0)
  {
    perror("[server]Eroare la write() catre client.\n");
    return;
  }
  else {
    printf("[Thread %d]Userul %s s-a logat\n",tdL.idThread, tdL.client->name);
    
    if(write(tdL.cl,&tdL.client->admin,sizeof(int)) <=0 )
    {
    perror("[server]Eroare la read() de la client.\n");
    return;
    }

    if(write(tdL.cl,&tdL.client->vote,sizeof(int)) <=0 )
    {
    perror("[server]Eroare la read() de la client.\n");
    return;
    }

    userMenu(tdL);
  }
  fflush(stdout); 
}

void inregistrare(thData tdL){
  if (strlen(tdL.client->name) > 0 && strlen(tdL.client->password) > 0)
  {
    return;
  }

  char username[30], parola[30];
  bzero(username, 30);
  if (read(tdL.cl, username, 30) <= 0)
  {
    perror("[server]Eroare la read() de la client.\n");
    return;
  }
  bzero(parola, 30);
  if (read(tdL.cl, parola, 30) <= 0)
  {
    perror("[server]Eroare la read() de la client.\n");
    return;
  }
  printf("[Thread %d]Inregistrare: %s\n", tdL.idThread, username);
  fflush(stdout);

  char* mesaj=insertUser(username,parola);
  if (write(tdL.cl, mesaj, 50) <= 0)
      {
        perror("[server]Eroare la write() catre client.\n");
        return;
      }
  else
      printf("[Thread %d]Userul %s s-a inregistrat.\n",tdL.idThread, username);
  
  fflush(stdout);

}

void adaugaAdmin(thData tdL){
  //update Users set admin=1 where username like '';
  //verificam daca userul este logat
  if (strlen(tdL.client->name) <= 0 && strlen(tdL.client->password) <= 0)
  {
    printf("[Thread %d]Accesare eronata pentru adaugare admin!\n",tdL.idThread);
    return;
  }

  char username[30], parola[30];
  bzero(username, 30);
  
  if (read(tdL.cl, username, 30) <= 0)
  {
    perror("[server]Eroare la read() de la client.\n");
    return;
  }

  char* mesaj=newAdmin(username);
  if (write(tdL.cl, mesaj, 50) <= 0)
      {
        perror("[server]Eroare la write() catre client.\n");
        return;
      }
  else
      printf("[Thread %d] Userul %s a fost promovat admin de catre %s.", tdL.idThread,username,tdL.client->name);

  fflush(stdout);
}

void sendUser(thData tdL)
{
  char name[30], password[30];
  printf("[Thread %d]Solicitare date: %s\n",tdL.idThread,tdL.client->name);
  fflush(stdout);
  strcpy(name, tdL.client->name);
  strcpy(password, tdL.client->password);

  if (write(tdL.cl, &name, 30) <= 0)
  {
    perror("[client]Eroare la write() spre client.\n");
    return;
  }

  if (write(tdL.cl, &password, 30) <= 0)
  {
    perror("[client]Eroare la write() spre client.\n");
    return;
  }
  
  if (write(tdL.cl, &tdL.client->vote, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre client.\n");
    return;
  }

  if (write(tdL.cl, &tdL.client->admin, sizeof(int)) <= 0)
  {
    perror("[client]Eroare la write() spre client.\n");
    return;
  }
  //printf("Am trimis\n");
}
void welcome(thData tdL)
{
  int nr;
  while (1)
  {
    if (read(tdL.cl, &nr, sizeof(int)) <= 0)
    {
      printf("[Thread %d]S-a intrerupt...", tdL.idThread);
      perror("Eroare la read() de la client.");
      break;
    }
    //printf ("[Thread %d]Mesajul a fost receptionat...%d\n",tdL.idThread, nr);

    if (nr == 0)
    {
      printf("[Thread %d]S-a deconectat...\n", tdL.idThread);
      free(tdL.client);
      fflush(stdout);
      break;
    }
    else
      parseaza(tdL, nr,"welcome");
  }
}

int main()
{
  struct sockaddr_in server; // structura folosita de server
  struct sockaddr_in from;
  int nr; //mesajul primit de trimis la client
  int sd; //descriptorul de socket
  int pid;
  pthread_t th[100]; //Identificatorii thread-urilor care se vor crea
  int i = 0;

  /* crearea unui socket */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[server]Eroare la socket().\n");
    return errno;
  }
  /* utilizarea optiunii SO_REUSEADDR */
  int on = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  /* pregatirea structurilor de date */
  bzero(&server, sizeof(server));
  bzero(&from, sizeof(from));

  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
  server.sin_family = AF_INET;
  /* acceptam orice adresa */
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  /* utilizam un port utilizator */
  server.sin_port = htons(PORT);

  /* atasam socketul */
  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[server]Eroare la bind().\n");
    return errno;
  }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen(sd, 2) == -1)
  {
    perror("[server]Eroare la listen().\n");
    return errno;
  }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
  {
    int client;
    thData *td; //parametru functia executata de thread
    int length = sizeof(from);

    printf("[server]Asteptam la portul %d...\n", PORT);
    fflush(stdout);

    // client= malloc(sizeof(int));
    /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
    if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
    {
      perror("[server]Eroare la accept().\n");
      continue;
    }

    /* s-a realizat conexiunea, se astepta mesajul */

    // int idThread; //id-ul threadului
    // int cl; //descriptorul intors de accept

    td = (struct thData *)malloc(sizeof(struct thData));
    td->idThread = i++;
    td->cl = client;
    td->client = (struct user *)malloc(sizeof(struct user));
    pthread_create(&th[i], NULL, &treat, td);

  } //while
};
static void *treat(void *arg)
{
  struct thData tdL;
  tdL = *((struct thData *)arg);
  printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
  fflush(stdout);
  pthread_detach(pthread_self());
  raspunde((struct thData *)arg);
  /* am terminat cu acest client, inchidem conexiunea */
  close((intptr_t)arg);
  return (NULL);
};

void raspunde(void *arg)
{
  int nr, i = 0;
  struct thData tdL;
  tdL = *((struct thData *)arg);

  welcome(tdL);
}