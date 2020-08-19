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
#include <mysql/mysql.h>

MYSQL* getConn();

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData
{
  int idThread; //id-ul thread-ului tinut in evidenta de acest program
  int cl;       //descriptorul intors de accept
} thData;
int welcome(struct thData sd);
int parsezComanda(char a[30])
{
  if (strcmp(a, "login") == 0)
    return 1;
  if (strcmp(a, "exit") == 0)
    return 0;
  if (strcmp(a, "inregistrare") == 0)
    return 2;
  return -1;
}

short checkUsername(char username[20]){
  MYSQL* conexiune=getConn();
  short rez=0;
  char interogare[200];
  strcpy(interogare,"select * from Users where username like '");
  strcat(interogare,username);
  strcat(interogare,"'");


  //executam interogarea
  int status=mysql_query(conexiune,interogare);
  if(status){
      printf("Could not execute statement(s)");
      mysql_close(conexiune);
      exit(0);
  }
  /* process each statement result */
  do {
  /* did current statement return data? */
  //PROCESAREA DATELOR
  MYSQL_RES *result = mysql_store_result(conexiune);
  if (result)
  {
      /* yes; process rows and free the result set */
      //process_result_set(conexiune, result);
      rez=mysql_num_rows(result); //nr de utilizatori cu acel username
      printf("%s",(char*)result);
      mysql_free_result(result);
  }
  else          /* no result set or error */
  {
      if (mysql_field_count(conexiune) == 0)
      {
      printf("%lld rows affected\n",mysql_affected_rows(conexiune));
      }
      else  /* some error occurred */
      {
      printf("Could not retrieve result set\n");
      break;
      }
  }
  /* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
  if ((status = mysql_next_result(conexiune)) > 0)
      printf("Could not execute statement\n");
  } while (status == 0);
  printf("rez=%d\n",rez);
  mysql_close(conexiune);
  return rez;
}
//returneaza o conexiune la BD
MYSQL* getConn(){
  MYSQL *conexiune=mysql_init(NULL);
  if(conexiune==NULL)
  {
      fprintf(stderr, "%s\n", mysql_error(conexiune));
      exit(1);
  }
  if (mysql_real_connect(conexiune, "localhost", "root", "elefant.2004", "proiect", 0, NULL, 0) == NULL) 
  {
      fprintf(stderr, "Am dat peste o eroare: %s\n", mysql_error(conexiune));
      mysql_close(conexiune);
      exit(1);
  }
  return conexiune;
}

int login(struct thData sd)
{
  
  MYSQL *conexiune = getConn();
  //printf("Conectare la baza de date: [ OK ]\n");
  char username[20]="";
  char parola[20]="";
  int length1=0,length2=0;
  printf("[login]1 INCEPUT\n");
  bzero(username,20);
  if(read(sd.cl,&username,sizeof(username))<=0)
    {perror("[Server][login] Eroare la read() de la client.\n"); close(sd.cl); return -1;}
  printf("[login]2 %s\n",username);
  length1=strlen(username);
  printf("[login]%d\n",length1);
  
  if(write(sd.cl,"ok",sizeof("ok"))<=0)
  {perror("[Server][login] Eroare la write() catre client.\n"); close(sd.cl); return -1;}
  
  bzero(parola,20);
  if( read(sd.cl,&parola,20) <= 0)
    {perror("[Server][login] Eroare la read() de la client.\n"); close(sd.cl); return -1;}
  printf("[login]3 %s\n",parola);
  length2=strlen(parola);

  printf("[login]4 %d\n",length2);
  //fflush(stdout);
  printf("[login]5 FFLUSH\n");
  char interogare[250]="";
  strcpy(interogare,"select * from Users where username like '");
  printf("[login]6 %s\n",interogare);

  //"select * from Users where username like 'maria' and password like 'maria99'";
  strcat(interogare,username);
  printf("[login]7 %s\n",interogare);
  strcat(interogare,"' and password like '");
  printf("[login]8 %s\n",interogare);
  strcat(interogare,parola);
  printf("[login]9 %s\n",interogare);
  strcat(interogare,"'");  
  printf("[login]10 %s\n",interogare);

  //executam interogarea
  int status=mysql_query(conexiune,interogare);
  printf("[login]11 %d\n",status);
  if(status){
      printf("Could not execute statement(s)");
      mysql_close(conexiune);
      exit(0);
  }
  printf("[login]12\n");
  /* process each statement result */
  do {
  /* did current statement return data? */
  //PROCESAREA DATELOR
  MYSQL_RES *result = mysql_store_result(conexiune);
  if (result)
  {
      /* yes; process rows and free the result set */
      //process_result_set(conexiune, result);
      //retinem numarul de randuri returnate; il verificam mai jos
      printf("[login]13 %d\n",mysql_field_count(conexiune));
      length1=mysql_num_rows(result);
      printf("[login]14 %d\n",length1);
      printf("[login]result = %s\n",(char*)result);
      printf("[login]15\n");
      // if( length1 > 0 )
        mysql_free_result(result);
  }
  else          /* no result set or error */
  {
      if (mysql_field_count(conexiune) == 0)
      {
      printf("%lld rows affected\n",
              mysql_affected_rows(conexiune));
      }
      else  /* some error occurred */
      {
      printf("Could not retrieve result set\n");
      break;
      }
  }
  /* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
  if ((status = mysql_next_result(conexiune)) > 0)
      printf("Could not execute statement\n");
  } while (status == 0);

  //mysql_close(conexiune);
  
  printf("[login]16 OUT OF WHILE\n");
  char rasp[3]="";
  printf("[login]19\n");
  //daca nr de randuri returnate este 
  if (length1 != 1) 
  {
    printf("[login]20\n");
    strcpy(rasp,"no");
    printf("[login]21\n");
    //fprintf(stderr, "[login]%s\n", mysql_errno(conexiune));
    printf("[login]22\n");
    printf("[login]m-am blocat aici\n");
    //mysql_close(conexiune);
    printf("[login]return 0\n");
  }
  else
   strcpy(rasp,"ok");
  printf("[login]17\n");
  fflush(stdout);
  printf("[login]18 rasp=%s\n",rasp);
  length1=write(sd.cl , &rasp , sizeof(rasp));
  if(length1<=0)
     {printf("[Server][login] Eroare la write() catre client.\n");close(sd.cl);}
  else {
    printf("[login]am scris %d biti - raspuns corect/incorect",length1);
    if(strcmp(rasp,"ok")==0){
      printf("[login]23\n");
      if(conexiune!=NULL) mysql_close(conexiune);
      printf("[login]24\n");
      printf("[login]Utilizatorul %s s-a conectat la server.\n",username);
      printf("[login]25\n");
      return 1;
      }
    else {
      printf("Eroare la conectare user : %s",username);
      }
    }
  printf("length1=%d",length1);
  if(conexiune!=NULL) mysql_close(conexiune);

  return 0;
}
int registration(struct thData sd)
{
  printf("[thread]- %d - inregistrare \n",sd.idThread);
  char username[20]="";
  char parola[20]="";

  while(1){
    printf("1");
  bzero(username,20);
      printf("2");
  fflush(stdout);
      printf("3");
  if(read(sd.cl,&username,sizeof(username))<=0)
    {perror("[Server][login] Eroare la read() de la client.\n"); close(sd.cl);}
  printf("%s\n",username);

  if(strcmp(username,"back")==0) welcome(sd);

  if(checkUsername(username)==1) //userul exista, trimitem raspuns la server
    {if(write(sd.cl,"no",sizeof("no"))<=0)
      {perror("[Server][login] Eroare la write() catre client.\n"); close(sd.cl);}}
  else{ //userul nu exista
    if(write(sd.cl,"ok",sizeof("ok"))<=0)
      {perror("[Server][login] Eroare la write() catre client.\n"); close(sd.cl);}
    break;
    }
  }

  bzero(parola,20);
  if( read(sd.cl,&parola,20) <= 0)
    {perror("[Server][login] Eroare la read() de la client.\n"); close(sd.cl);}
  fflush(stdout);
  printf("Am inregistrat pe %s, Yeehaw",username);

  return 2;
}

int welcome(struct thData sd)
{
  printf("[welcome]Astept comanda:\n");
  char comanda[30]="";
  int a,b;
  printf("[welcome]1\n");
  fflush(stdout);
  printf("[welcome]2\n");
  bzero(comanda,30);
  printf("[welcome]3\n");
  if (read(sd.cl, &comanda, sizeof(comanda)) <= 0)
  {
    printf("[welcome]Eroare la citire de la client.\n");
    return errno;
  }
  else
  {
    a = parsezComanda(comanda);
    printf("[welcome]4\n");
    printf("[welcome]Ati introdus comanda %s, cod %ls\n",comanda,&a);
    if ((b=write(sd.cl, &a , sizeof(a))) <= 0)
    {
      perror("Eroare la scriere catre client.\n");
      return errno;
    }
    else{
      printf("[welcome]b=%d \n",b);
      printf("[welcome]5\n");
      switch (a)
      {
      case 0:
        printf("Clientul %d s-a deconectat.\n", sd.idThread);
        a=0;
        //close(sd.cl);
        break;
      case 1:
        a = login(sd);
        break;
      case 2:
        a = registration(sd);
        break;
      default:
        printf("[server][welcome]Eroare la parsarea comenzii %s, client %d", comanda, sd.idThread);
        a=-1;
        //close(sd.cl);
        return errno; break;
      }
    }
    // if (a < 0)
    // {
    //   printf("[server]Eroare la parsarea comenzii %s, client %d", comanda, sd.idThread);
    //   close(sd.cl);
    //   return errno;
    // }
  }
  return a;
}

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);

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
  int a = 1;
  while (a>0){
    printf("a mers,super!\n");
    a=welcome(tdL);
  }
  close(tdL.cl);
  
}
