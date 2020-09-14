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

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl; //descriptorul intors de accept
  struct user* client;
}thData;

typedef struct user{
    char name[30];
    char password[30];
}user;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
void welcome(thData tdL);
void parseaza(thData tdL,int cod);
void login(thData tdL);
void getUser(thData tdL,char username[30],char password[30]);
void sendUser(thData tdL);

void parseaza(thData tdL,int cod){
    if(cod == 1) sendUser(tdL);
    else if(cod == 2) login(tdL);
    else return;
}

void login(thData tdL){
    char username[30],parola[30];
    bzero(username,30);
    if(read(tdL.cl,username,30)<=0){
        perror ("[server]Eroare la read() de la client.\n"); return;
    }
    bzero(parola,30);
    if(read(tdL.cl,parola,30)<=0){
        perror ("[server]Eroare la read() de la client.\n"); return;
    }
    printf("[Thread %d]am primit mesajele: %s , %s. Verificam...\n",tdL.idThread,username,parola);
    fflush(stdout);

    //VERIFICARE + RASPUNS
    getUser(tdL,username,parola);
    printf("%s %s\n",tdL.client->name,tdL.client->password);
    if( tdL.client->name != NULL)
        {
          if(write(tdL.cl,"ok",10)<=0) {perror ("[server]Eroare la write() catre client.\n"); return;}
          else //userMenu(tdL);
          printf("userul %s s-a logat\n",tdL.client->name);
          fflush(stdout);
        }
      else
      {
        if(write(tdL.cl,"not ok",10)<=0) {perror ("[server]Eroare la write() catre client.\n"); return;}
      }        

}

//getUser from file
void getUser(thData tdL,char username[30],char password[30]){
    FILE* fisier = fopen("login.txt","rw");
    size_t len1=0,len2=0;
    char *vpassword=NULL, *vusername=NULL;
    int usr=0,pas=0;
    //struct user* client=(struct user*)malloc(sizeof(struct user));

    if(strlen(username) <= 0 && strlen(password) <= 0){
        fclose(fisier);
        free(vusername);free(vpassword);
        return ;
    }

    if (fisier == NULL)
    {
        perror("[Login] Eroare la deschiderea fisierului.\n");
        free(vusername);free(vpassword);
        exit(1);
    }
    //else printf("Am deschis fisierul corect \n");
    
    usr=getline(&vusername, &len1, fisier);
    pas=getline(&vpassword, &len2, fisier);
    //printf("%ld %ld\n",strlen(username),strlen(password));
    while (usr > 0 && pas > 0)
        {
            if(vusername[strlen(vusername)-1]=='\n')
                vusername[strlen(vusername)-1]='\0';
            if(vpassword[strlen(vpassword)-1]=='\n')
                vpassword[strlen(vpassword)-1]='\0';
            //  printf("%s %s\n",vusername,vpassword);
            //  printf("%ld %ld\n",strlen(vusername),strlen(vpassword));
            //  printf("comparam: %d %d\n",strcmp(username, vusername),strcmp(password,vpassword));
            if (strcmp(username, vusername) == 0 && strcmp(password,vpassword) == 0)
            {
                strcpy(tdL.client->name,vusername);              
                strcpy(tdL.client->password,vpassword);
                break;
            }
            usr=getline(&vusername, &len1, fisier);
            pas=getline(&vpassword, &len2, fisier);
        }
    fclose(fisier);
    free(vusername);free(vpassword);
}

void sendUser(thData tdL){
   char name[30],password[30];
   fflush(stdout);
   printf("name: %s\n",tdL.client->name);
   strcpy(name,tdL.client->name);
   strcpy(password,tdL.client->password);

  if(write(tdL.cl,&name,30) <= 0){
    perror ("[client]Eroare la write() spre client.\n");
    return;
  }

  if(write(tdL.cl,&password,30) <= 0){
    perror ("[client]Eroare la write() spre client.\n");
    return;
  }

}
void welcome(thData tdL){
    int nr;
    while(1){
      if (read (tdL.cl, &nr,sizeof(int)) <= 0)
        {
          printf("[Thread %d]\n",tdL.idThread);
          perror ("Eroare la read() de la client.\n");
          break;
        }
      //printf ("[Thread %d]Mesajul a fost receptionat...%d\n",tdL.idThread, nr);

      if(nr==0){printf("[Thread %d]S-a deconectat...\n",tdL.idThread); free(tdL.client); fflush(stdout); break;}
      else parseaza(tdL,nr);
    }
}

int main ()
{
  struct sockaddr_in server;	// structura folosita de server
  struct sockaddr_in from;	
  int nr;		//mesajul primit de trimis la client 
  int sd;		//descriptorul de socket 
  int pid;
  pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
	int i=0;
  

  /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));
  bzero (&from, sizeof (from));
  
  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }
  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
    {
      int client;
      thData * td; //parametru functia executata de thread     
      int length = sizeof (from);

      printf ("[server]Asteptam la portul %d...\n",PORT);
      fflush (stdout);

      // client= malloc(sizeof(int));
      /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
      if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
	{
	  perror ("[server]Eroare la accept().\n");
	  continue;
	}
	
        /* s-a realizat conexiunea, se astepta mesajul */
    
	// int idThread; //id-ul threadului
	// int cl; //descriptorul intors de accept

	td=(struct thData*)malloc(sizeof(struct thData));	
	td->idThread=i++;
	td->cl=client;
  td->client=(struct user*)malloc(sizeof(struct user));
	pthread_create(&th[i], NULL, &treat, td);	      
				
	}//while    
};				
static void *treat(void * arg)
{		
		struct thData tdL; 
		tdL= *((struct thData*)arg);	
		printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
		fflush (stdout);		 
		pthread_detach(pthread_self());		
		raspunde((struct thData*)arg);
		/* am terminat cu acest client, inchidem conexiunea */
		close ((intptr_t)arg);
		return(NULL);	
  		
};


void raspunde(void *arg)
{
    int nr, i=0;
	struct thData tdL; 
	tdL= *((struct thData*)arg);
	
    welcome(tdL);

}