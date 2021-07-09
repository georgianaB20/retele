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
#include <ctype.h>

/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

char* parola_db="your_password";
char* database="your_database";

typedef struct thData
{
	int idThread; //id-ul thread-ului tinut in evidenta de acest program
	int cl;		  //descriptorul intors de accept
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

typedef struct comment
{
	char text[250];
	char user[30];
	int song_id;
} comment;

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

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);
void welcome(thData tdL);
void parseaza(thData tdL, int cod, char from[10]);
void login(thData tdL);
char *getUser(thData tdL, char username[30], char password[30]);
int sendUser(thData tdL);
int inregistrare(thData tdL);
int adaugaAdmin(thData tdL);
int banUserVote(thData tdL);
char *banUser(char username[30]);
void addSong_C(thData tdL);
char *addSong_BD(song *s, char *genres);
void addGenre(int id, char genre[10], MYSQL *conn);
song *getSongByNameArtist(char name[30], char artist[30]);
song *getSongById(int id);
char *getTop_BD(char *type);
void getTop(thData tdL);
void cautaCantec(thData tdL);
char *adaugaVot(thData tdL, float vot, int song_id);
char *adaugaComentariu(thData tdL, char comentariu[100], int song_id);
char *deleteSong(char name[30], char artist[30]);
void deleteSong_C(thData tdL);

MYSQL *getConnection()
{
	MYSQL *conn = mysql_init(NULL);
	if (conn == NULL)
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		exit(1);
	}
	if (mysql_real_connect(conn, "localhost", "root", parola_db, database, 0, NULL, 0) == NULL)
	{
		fprintf(stderr, "Am dat peste o eroare: %s\n", mysql_error(conn));
		mysql_close(conn);
		exit(1);
	}
	//printf("Conectare la baza de date: [ OK ]\n");
	return conn;
}

char *adaugaVot(thData tdL, float vot, int song_id)
{
	char *mesaj;
	MYSQL *conn = getConnection();
	if (conn == NULL)
	{
		mesaj = "Baza de date indisponibila.Incearca din nou.\n";
		return mesaj;
	}

	char *mesaj2 = getUser(tdL, tdL.client->name, tdL.client->password);
	if (strcmp(mesaj2, "Salut! Te-ai logat cu succes.\n") == 0 && tdL.client->vote == 0)
	{
		mesaj = "Dreptul tau de vot a fost restrictionat.\n";
		mysql_close(conn);
		return mesaj;
	}

	char id[10], vote[10];
	sprintf(id, "%d", song_id);
	sprintf(vote, "%2f", vot);

	char stmt[250] = "INSERT INTO Votes (username,song_id,value) VALUES (\"";
	strcat(stmt, tdL.client->name);
	strcat(stmt, "\",");
	strcat(stmt, id);
	strcat(stmt, ",");
	strcat(stmt, vote);
	strcat(stmt, ")");

	if (mysql_query(conn, stmt) != 0)
	{
		if (mysql_errno(conn) == 1062)
			mesaj = "Ai votat deja.\n";
		else
			mesaj = "Eroare la baza de date. Incearca din nou.\n";

		mysql_close(conn);
		return mesaj;
	}
	else
	{
		mysql_close(conn);
		return "Vot inregistrat!\n";
	}
}

char *deleteSong(char name[30], char artist[30])
{
	MYSQL *conn = getConnection();
	song *s = (struct song *)malloc(sizeof(struct song));
	char *mesaj;
	if (conn == NULL)
	{
		return "Baza de date indisponibila.Incearca din nou.\n";
		//return NULL;
	}

	char interogare[200] = "delete from Songs where name like lower(\"";
	strcat(interogare, name);
	strcat(interogare, "\") and artist like lower(\"");
	strcat(interogare, artist);
	strcat(interogare, "\")");
	//printf("%s", interogare);

	if (mysql_query(conn, interogare) != 0)
	{
		if (mysql_affected_rows(conn) == 0)
		{
			mesaj = "Melodia nu exista\n";
			mysql_close(conn);
			return mesaj;
		}
	}
	else
	{
		if (mysql_affected_rows(conn) >= 1)
			mesaj = "Melodia a fost stearsa!\n";
	}
	mysql_close(conn);
	return mesaj;
}

char *adaugaComentariu(thData tdL, char comentariu[100], int song_id)
{
	char *mesaj;
	MYSQL *conn = getConnection();
	if (conn == NULL)
	{
		mesaj = "Baza de date indisponibila.Incearca din nou.\n";
		return mesaj;
	}

	char id[10];
	sprintf(id, "%d", song_id);
	//printf("%s",id);

	char stmt[250] = "INSERT INTO Comments (username,song_id,comment) VALUES (\"";
	strcat(stmt, tdL.client->name);
	strcat(stmt, "\",");
	strcat(stmt, id);
	strcat(stmt, ",\"");
	strcat(stmt, comentariu);
	strcat(stmt, "\")");

	if (mysql_query(conn, stmt) != 0)
	{
		mesaj = "Eroare la baza de date. Incearca din nou.\n";
		mysql_close(conn);
		return mesaj;
	}
	else
	{
		mysql_close(conn);
		return "Comentariu adaugat!\n";
	}
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

char *getTop_BD(char *type)
{
	char *mesaj;
	char *top = malloc(1024);
	bzero(top, 1024);

	MYSQL *conn = getConnection();
	if (conn == NULL)
	{
		return "Baza de date indisponibila.Incearca din nou.\n";
	}

	char stmt[250] = "";
	if (strcmp(type, "general") == 0)
	{
		strcat(stmt, "select s.id,avg(v.value) from Songs s join Votes v on s.id=v.song_id group by s.id order by 2 desc");
	}
	else
	{
		strcat(stmt, "select s.id,avg(v.value) from Songs s join Votes v on s.id=v.song_id join Genres g on s.id=g.song_id where g.genre like LOWER(\'");
		strcat(stmt, type);
		strcat(stmt, "\') group by s.id order by 2 desc");
	}

	//executam interogarea:
	if (mysql_query(conn, stmt) != 0)
	{
		if (mysql_affected_rows(conn) == 0)
			strcat(top, "Valori incorecte pentru top.\n");
	}
	else
	{
		MYSQL_RES *res = mysql_store_result(conn);
		if (res == NULL)
			strcat(top, "Valori incorecte pentru top.\n");
		int n = mysql_num_rows(res);
		//luam doar primele 10 melodii din top
		if (n > 10)
			n = 10;
		strcat(top, "\nArtist \t \t Song \t \t Rating \t \n");
		for (int i = 0; i < n; i++)
		{
			MYSQL_ROW row = mysql_fetch_row(res);
			int id = atoi(row[0]);
			song *s = getSongById(id);
			strcat(top, s->artist);
			strcat(top, "\t");
			strcat(top, s->name);
			strcat(top, "\t");
			strcat(top, row[1]);
			strcat(top, "\n");
		}
		mysql_free_result(res);
	}
	mysql_close(conn);
	return top;
}

char *getComments_BD(int song_id)
{
	MYSQL *conn = getConnection();

	if (conn == NULL)
	{
		return "Baza de date indisponibila.Incearca din nou.\n";
	}

	char *comentarii = malloc(1024);
	bzero(comentarii, 1024);

	char comments[100] = "select * from Comments where song_id=";
	char nr[10] = "";
	sprintf(nr, "%d", song_id);
	strcat(comments, nr);

	if (mysql_query(conn, comments) != 0)
	{
		mysql_close(conn);
		return "Eroare la baza de date.\n";
	}
	else
	{
		MYSQL_RES *res3 = mysql_store_result(conn);

		if (res3 == NULL)
		{
			mysql_close(conn);
			return "--------------Nu exista comentarii--------------\n";
		}

		if (mysql_num_rows(res3) == 0)
		{
			mysql_free_result(res3);
			mysql_close(conn);
			return "--------------Nu exista comentarii--------------\n";
		}

		MYSQL_ROW rows;
		strcat(comentarii, "\t\t\tComentarii\n");
		while ((rows = mysql_fetch_row(res3)) != NULL)
		{
			strcat(comentarii, rows[2]);
			strcat(comentarii, ":");
			strcat(comentarii, rows[3]);
			strcat(comentarii, "\n");
		}
		mysql_free_result(res3);
	}
	mysql_close(conn);
	return comentarii;
}

char *getRating(int song_id)
{
	char *mesaj = malloc(10);
	MYSQL *conn = getConnection();
	if (conn == NULL)
	{
		//return "Baza de date indisponibila.Incearca din nou.\n";
		mesaj = "0.0";
		return mesaj;
	}

	char votes[100] = "select avg(value) from Votes where song_id=";
	char *rating = malloc(10);
	char nr[10] = "";
	sprintf(nr, "%d", song_id);
	strcat(votes, nr);
	if (mysql_query(conn, votes) != 0)
	{
		printf("Eroare preluare voturi.\n");
		fflush(stdout);
		mysql_close(conn);
		mesaj = "0.0";
		return mesaj;
	}
	else
	{

		MYSQL_RES *res2 = mysql_store_result(conn);

		//  printf("%ld",mysql_num_rows(res2));
		//  fflush(stdout);
		if (mysql_num_rows(res2) > 0)
		{
			MYSQL_ROW row = mysql_fetch_row(res2);
			// printf("row[0]=%s",row[0]);
			// fflush(stdout);
			if (row[0]==NULL){
				mysql_free_result(res2);
				mysql_close(conn);
				bzero(mesaj,10);
				mesaj = "0.0";
				return mesaj;
			}
			else{
			bzero(rating, 10);
			strcpy(rating, row[0]);
			mysql_free_result(res2);
			}
		}
		else
		{
			mysql_free_result(res2);
			mysql_close(conn);
			bzero(mesaj,10);
			mesaj = "0.0";
			return mesaj;
		}
	}
	mysql_close(conn);
	return rating;
}

song *getSong_BD(char name[50], char artist[50])
{
	MYSQL *conn = getConnection();
	song *s = (struct song *)malloc(sizeof(struct song));
	bzero(s, sizeof(struct song));
	if (conn == NULL)
	{
		//return "Baza de date indisponibila.Incearca din nou.\n";
		//return NULL;
		//printf("Bad stuff\n");
		s->id = 0;
		return s;
	}

	char interogare[200] = "select * from Songs where name like lower(\"";
	strcat(interogare, name);
	strcat(interogare, "\") and artist like lower(\"");
	strcat(interogare, artist);
	strcat(interogare, "\")");
	//printf("%s\n", interogare);

	if (mysql_query(conn, interogare) != 0)
	{
		if (mysql_affected_rows(conn) == 0)
		//mesaj = "Melodia nu exista.\n";
		{
			//printf("1affected rows %d\n",(int)mysql_affected_rows(conn));
			mysql_close(conn);
			s->id = 0;
			return s;
		}
	}
	else
	{
		//printf("2affected rows %d\n",(int)mysql_affected_rows(conn));
		// if ((int)mysql_affected_rows(conn) > 0)
		// {
		//printf("affected rows %d\n",(int)mysql_affected_rows(conn));
		MYSQL_RES *res = mysql_store_result(conn);
		if (res == NULL)
		//mesaj = "Id gresit\n";
		{
			mysql_close(conn);
			s->id = 0;
			return s;
		}

		int nr_rows = mysql_num_rows(res);
		// printf("nr cantece:%d\n",nr_rows);
		// fflush(stdout);
		MYSQL_ROW rand;
		if (nr_rows >= 1)
		{
			rand = mysql_fetch_row(res);
			//bzero(s,sizeof(struct song));
			//printf("%s %s %s %s %s\n", rand[0], rand[1], rand[2], rand[3], rand[4]);
			s->id = atoi(rand[0]);
			strcpy(s->name, rand[1]);
			strcpy(s->artist, rand[2]);
			strcpy(s->link, rand[3]);
			strcpy(s->description, rand[4]);
			mysql_free_result(res);
			//printf("%d\n%s\n%s\n%s\n%s\n1\n",s->id,s->name,s->artist,s->link,s->description);
		}
		//}
		// else
		// 	s->id = 0;
	}
	mysql_close(conn);
	return s;
}

song *mostRecentSong()
{
	//char *mesaj;
	song *s = (struct song *)malloc(sizeof(struct song));
	MYSQL *conn = getConnection();
	if (conn == NULL)
	{
		//return "Baza de date indisponibila.Incearca din nou.\n";
		free(s);
		return NULL;
	}

	char stmt[100] = "SELECT MAX(id) FROM Songs";

	if (mysql_query(conn, stmt) != 0)
	{
		if (mysql_affected_rows(conn) == 0)
			//mesaj = "User sau parola incorecte.\n";
			free(s);
	}
	else
	{
		MYSQL_RES *res = mysql_store_result(conn);
		if (res == NULL)
			//mesaj = "User sau parola incorecte.\n";
			free(s);

		int n = mysql_num_rows(res);
		if (n == 1)
		{
			MYSQL_ROW row = mysql_fetch_row(res);
			s->id = atoi(row[0]);
			mysql_free_result(res);
			//mesaj = "Salut! Te-ai logat cu succes.\n";
		}
		else
		{
			free(s);
			//mesaj = "Eroare la baza de date.\n";
		}
		mysql_close(conn);
		return s;
	}
}

char *newAdmin(char username[30])
{
	char *mesaj;
	MYSQL *conn = getConnection();
	if (conn == NULL)
	{
		return "Baza de date indisponibila.Incearca din nou.\n";
	}

	char stmt[250] = "UPDATE Users SET admin=1 WHERE username like \"";
	strcat(stmt, username);
	strcat(stmt, "\"");
	printf("%s", stmt);

	if (mysql_query(conn, stmt) != 0)
	{
		if (mysql_affected_rows(conn) == 0)
			mesaj = "Userul nu exista.\n";
		else
		{
			mesaj = "Eroare la baza de date.\n";
			mysql_rollback(conn);
		}

		mysql_close(conn);
		return mesaj;
	}
	else
	{
		mysql_close(conn);
		return "Userul a fost promovat admin!\n";
	}
}

char *getUser(thData tdL, char username[30], char password[30])
{
	char *mesaj;
	MYSQL *conn = getConnection();
	if (conn == NULL)
	{
		return "Baza de date indisponibila.Incearca din nou.\n";
	}

	char stmt[250] = "SELECT * FROM Users WHERE username like \"";
	strcat(stmt, username);
	strcat(stmt, "\" and password like \"");
	strcat(stmt, password);
	strcat(stmt, "\"");

	if (mysql_query(conn, stmt) != 0)
	{
		if (mysql_affected_rows(conn) == 0)
			mesaj = "User sau parola incorecte.\n";
	}
	else
	{
		MYSQL_RES *res = mysql_store_result(conn);
		if (res == NULL)
			mesaj = "User sau parola incorecte.\n";

		int n = mysql_num_rows(res);
		if (n == 1)
		{
			MYSQL_ROW row = mysql_fetch_row(res);
			strcpy(tdL.client->name, row[0]);
			strcpy(tdL.client->password, row[1]);
			tdL.client->admin = atoi(row[2]);
			tdL.client->vote = atoi(row[3]);
			mysql_free_result(res);
			mesaj = "Salut! Te-ai logat cu succes.\n";
		}
		else
		{
			if (n == 0)
				mesaj = "User sau parola incorecte.\n";
			else
				mesaj = "Eroare la baza de date.\n";
		}
		mysql_close(conn);
		return mesaj;
	}
}

char *banUser(char username[30])
{
	char *mesaj;
	MYSQL *conn = getConnection();
	if (conn == NULL)
	{
		return "Baza de date indisponibila.Incearca din nou.\n";
	}

	char stmt[250] = "UPDATE Users SET vote=0 WHERE username like \"";
	strcat(stmt, username);
	strcat(stmt, "\"");
	printf("%s", stmt);

	if (mysql_query(conn, stmt) != 0)
	{
		if (mysql_affected_rows(conn) == 0)
			mesaj = "Userul nu exista.\n";
		else
		{
			mesaj = "Eroare la baza de date.\n";
			mysql_rollback(conn);
		}

		mysql_close(conn);
		return mesaj;
	}
	else
	{
		mysql_close(conn);
		return "Dreptul de vot al userului a fost restrictionat!\n";
	}
}

song *getSongById(int id)
{
	char *mesaj;
	song *s = (struct song *)malloc(sizeof(struct song));
	MYSQL *conn = getConnection();
	if (conn == NULL)
	{
		printf("Bad stuff\n");
		//return "Baza de date indisponibila.Incearca din nou.\n";
	}

	char stmt[250] = "SELECT * FROM Songs WHERE id=";
	char nr[10] = "";
	//itoa(id,nr,10);
	sprintf(nr, "%d", id);
	strcat(stmt, nr);

	if (mysql_query(conn, stmt) != 0)
	{
		if (mysql_affected_rows(conn) == 0)
			//mesaj = "Melodia nu exista.\n";
			free(s);
	}
	else
	{
		MYSQL_RES *res = mysql_store_result(conn);
		if (res == NULL)
			//mesaj = "Id gresit\n";
			free(s);

		int n = mysql_num_rows(res);
		if (n == 1)
		{
			MYSQL_ROW row = mysql_fetch_row(res);
			strcpy(s->name, row[1]);
			strcpy(s->artist, row[2]);
			strcpy(s->link, row[3]);
			strcpy(s->description, row[4]);
			s->id = atoi(row[0]);
			mysql_free_result(res);
			//mesaj = "Salut! Te-ai logat cu succes.\n";
		}
		else
		{
			//mesaj = "Eroare la baza de date.\n";
			free(s);
		}
		mysql_close(conn);
		return s;
	}
}

char *addSong_BD(song *s, char *genres)
{
	char *mesaj;
	MYSQL *conn = getConnection();
	if (conn == NULL)
	{
		mesaj = "Baza de date indisponibila.Incearca din nou.\n";
		return mesaj;
	}

	char stmt[250] = "INSERT INTO Songs (name,artist,description,link) VALUES (LOWER(\"";
	strcat(stmt, s->name);
	strcat(stmt, "\"),LOWER(\"");
	strcat(stmt, s->artist);
	strcat(stmt, "\"),\"");
	strcat(stmt, s->description);
	strcat(stmt, "\",\"");
	strcat(stmt, s->link);
	strcat(stmt, "\")");
	printf("\n%s\n", stmt);
	fflush(stdout);

	if (mysql_query(conn, stmt) != 0)
	{
		mesaj = "Eroare la baza de date. Incearca din nou.\n";
		printf("1");
		mysql_close(conn);
		return mesaj;
	}
	else
	{
		song *last = mostRecentSong();
		if (strchr(genres, ',') == NULL)
			addGenre(last->id, genres, conn);
		//genres[strlen(genres)-1]='\0';
		else
		{
			char *gen = strtok(genres, ",");
			gen[strlen(gen)] = '\0';
			printf("%s\n", gen);
			fflush(stdout);
			printf("%d %s\n", last->id, gen);
			fflush(stdout);
			if (last != NULL)
			{
				while (gen != NULL)
				{
					if (gen == NULL)
						break;
					addGenre(last->id, gen, conn);
					gen = strtok(NULL, ",");
				}
			}
		}
		//printf("Am terminat\n");

		mysql_close(conn);
		return "Melodie adaugata cu succes!\n";
	}
}

void addGenre(int id, char genre[30], MYSQL *conn)
{
	// MYSQL *conn = getConnection();
	if (conn != NULL)
	{
		char stmt[250] = "INSERT INTO Genres (song_id,genre) VALUES (";
		char nr[10] = "";
		//printf("9");
		sprintf(nr, "%d", id);
		strcat(stmt, nr);
		//printf("8");
		strcat(stmt, ",LOWER(\"");
		//printf("7");
		strcat(stmt, genre);
		strcat(stmt, "\"))");
		// printf("\n%s\n", stmt);
		// fflush(stdout);

		if (mysql_query(conn, stmt) != 0)
		{
			printf("AddGenre:Eroare la baza de date.\n");
			//mysql_close(conn);
		}
		// else
		// {
		// 	printf("[]Gen %s adaugat.\n", genre);
		// 	//mysql_close(conn);
		// }
	}
	return;
}

void parseaza(thData tdL, int cod, char from[10])
{
	//printf("%d",cod);
	if (strcmp(from, "userMenu") == 0)
	//tratam comenzile din userMenu
	{
		if (tdL.client->admin == 1)
		{
			if (cod == 1)
				sendUser(tdL);
			else if (cod == 2)
				adaugaAdmin(tdL);
			else if (cod == 3)
				banUserVote(tdL);
			else if (cod == 4)
				addSong_C(tdL);
			else if (cod == 5)
				getTop(tdL);
			else if (cod == 6)
				cautaCantec(tdL);
			else if (cod == 7)
				deleteSong_C(tdL);
			else
			{
				printf("Cod gresit1 %d\n", cod);
				return;
			}
		}
		else
		{
			if (cod == 1)
				sendUser(tdL);
			else if (cod == 2)
				addSong_C(tdL);
			else if (cod == 3)
				getTop(tdL);
			else if (cod == 4)
				cautaCantec(tdL);
			else
			{
				printf("Cod gresit2 %d\n", cod);
				return;
			}
		}
	}
	else if (strcmp(from, "welcome") == 0)
	//tratam comenzile din welcome
	{
		if (cod == 1)
			login(tdL);
		else if (cod == 2)
			inregistrare(tdL);
		else
		{
			printf("Cod gresit3 %d\n", cod);
			return;
		}
	}
	return;
}

void userMenu(thData tdL)
{
	int nr;
	while (1)
	{
		//printf("[userMenu]\n");
		if (read(tdL.cl, &nr, sizeof(int)) <= 0)
		{
			printf("[Thread %d]S-a intrerupt...", tdL.idThread);
			perror("Eroare la read() de la client.");
			break;
		}
		//printf ("[Thread %d]Mesajul a fost receptionat...%d\n",tdL.idThread, nr);

		if (nr == 0)
		{
			printf("[Thread %d]S-a delogat: %s\n", tdL.idThread, tdL.client->name);
			bzero(tdL.client, sizeof(tdL.client));
			fflush(stdout);
			//welcome(tdL);
			break;
		}
		else
			parseaza(tdL, nr, "userMenu");
	}
	return;
}

int banUserVote(thData tdL)
{

	if (strlen(tdL.client->name) <= 0 && strlen(tdL.client->password) <= 0)
	{
		printf("[Thread %d]Accesare eronata pentru adaugare admin!\n", tdL.idThread);
		return -1;
	}

	char username[30], parola[30];
	bzero(username, 30);

	if (read(tdL.cl, username, 30) <= 0)
	{
		perror("[server]Eroare la read() de la client.\n");
		return -1;
	}
	printf("[Thread %d] %s restrictioneaza vot %s\n", tdL.idThread, tdL.client->name, username);

	char *mesaj = banUser(username);
	if (write(tdL.cl, mesaj, 50) <= 0)
	{
		perror("[server]Eroare la write() catre client.\n");
		return -1;
	}
	else
		printf("[Thread %d] %s\n", tdL.idThread, mesaj);
	//printf("[Thread %d] Userul %s a fost promovat admin de catre %s.", tdL.idThread, username, tdL.client->name);

	fflush(stdout);
	return 3;
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
	char *mesaj = getUser(tdL, username, parola); //verific datele
	if (write(tdL.cl, mesaj, 50) <= 0)			  //trimit raspuns
	{
		perror("[server]Eroare la write() catre client.\n");
		return;
	}

	if (strcmp(mesaj, "Salut! Te-ai logat cu succes.\n") == 0)
	{
		printf("[Thread %d]Userul %s s-a logat\n", tdL.idThread, tdL.client->name);
		//trimitem datele la client
		if (write(tdL.cl, &tdL.client->admin, sizeof(int)) <= 0)
		{
			perror("[server]Eroare la write() catre client.\n");
			return;
		}

		if (write(tdL.cl, &tdL.client->vote, sizeof(int)) <= 0)
		{
			perror("[server]Eroare la read() de la client.\n");
			return;
		}
		fflush(stdout);
		userMenu(tdL);
	}
	else
	{ //resetam structura de client
		printf("Eroare login: %s %s", tdL.client->name, mesaj);
		bzero(tdL.client, sizeof(tdL.client));
		fflush(stdout);
		return;
	}
}

int inregistrare(thData tdL)
{
	if (strlen(tdL.client->name) > 0 && strlen(tdL.client->password) > 0)
	{
		return -1;
	}

	char username[30], parola[30];
	bzero(username, 30);
	if (read(tdL.cl, username, 30) <= 0)
	{
		perror("[server]Eroare la read() de la client.\n");
		return -1;
	}
	bzero(parola, 30);
	if (read(tdL.cl, parola, 30) <= 0)
	{
		perror("[server]Eroare la read() de la client.\n");
		return -1;
	}
	printf("[Thread %d]Inregistrare: %s\n", tdL.idThread, username);
	fflush(stdout);

	char *mesaj = insertUser(username, parola);
	if (write(tdL.cl, mesaj, 50) <= 0)
	{
		perror("[server]Eroare la write() catre client.\n");
		return -1;
	}
	else
		printf("[Thread %d]Userul %s s-a inregistrat.\n", tdL.idThread, username);

	fflush(stdout);
	return 2;
}

int adaugaAdmin(thData tdL)
{
	//update Users set admin=1 where username like '';
	//verificam daca userul este logat
	if (strlen(tdL.client->name) <= 0 && strlen(tdL.client->password) <= 0)
	{
		printf("[Thread %d]Accesare eronata pentru adaugare admin!\n", tdL.idThread);
		return -1;
	}

	char username[30], parola[30];
	bzero(username, 30);

	if (read(tdL.cl, username, 30) <= 0)
	{
		perror("[server]Eroare la read() de la client.\n");
		return -1;
	}

	char *mesaj = newAdmin(username);
	if (write(tdL.cl, mesaj, 50) <= 0)
	{
		perror("[server]Eroare la write() catre client.\n");
		return -1;
	}
	else
		printf("[Thread %d] %s", tdL.idThread, mesaj);
	//printf("[Thread %d] Userul %s a fost promovat admin de catre %s.", tdL.idThread, username, tdL.client->name);

	fflush(stdout);
	return 2;
}

void deleteSong_C(thData tdL)
{
	printf("[Thread %d]Sterge melodie: %s\n", tdL.idThread, tdL.client->name);

	char name[50], artist[50];
	bzero(name, 50);
	bzero(artist, 50);

	if (read(tdL.cl, name, 50) <= 0)
	{
		perror("[server]Eroare la read() de la client.\n");
		return;
	}

	if (read(tdL.cl, artist, 50) <= 0)
	{
		perror("[server]Eroare la read() de la client.\n");
		return;
	}

	char *rasp = deleteSong(name, artist);
	if (write(tdL.cl, rasp, 50) <= 0)
	{
		perror("[server]Eroare la write() catre client.\n");
		return;
	}
	else
		printf("[Thread %d] %s", tdL.idThread, rasp);
	//printf("[Thread %d] Userul %s a fost promovat admin de catre %s.", tdL.idThread, username, tdL.client->name);

	fflush(stdout);
}

void cautaCantec(thData tdL)
{
	printf("[Thread %d]Cauta melodie: %s\n", tdL.idThread, tdL.client->name);

	char name[50], artist[50];
	bzero(name, 50);
	bzero(artist, 50);
	// printf("0");
	// fflush(stdout);
	if (read(tdL.cl, name, 50) <= 0)
	{
		perror("[server]Eroare la read() de la client.\n");
		return;
	}
	//printf("7");
	//fflush(stdout);
	if (read(tdL.cl, artist, 50) <= 0)
	{
		perror("[server]Eroare la read() de la client.\n");
		return;
	}
	//printf("1 %s %s", artist, name);
	//fflush(stdout);

	song *s = getSong_BD(name, artist);
	//printf("%s\n", s->artist);
	fflush(stdout);
	//mergem mai departe daca cantecul exista

	if (write(tdL.cl, &s->id, sizeof(int)) <= 0)
	{
		perror("[server]Eroare la write() catre client.\n");
		return;
	}
	if (s->id != 0)
	{
		char *comentarii = getComments_BD(s->id);
		// printf("%s\n", comentarii);
		// fflush(stdout);
		char *rating = getRating(s->id);
		// printf("ceva\n");
		// fflush(stdout);
		long unsigned int lungime;
		// printf("2");

		// fflush(stdout);

		//write(tdL.cl,&lungime,sizeof(lungime));
		if (write(tdL.cl, s->artist, 50) <= 0)
		{
			perror("[server]Eroare la write() catre client.\n");
			return;
		}

		if (write(tdL.cl, s->name, 50) <= 0)
		{
			perror("[server]Eroare la write() catre client.\n");
			return;
		}

		if (write(tdL.cl, s->description, 100) <= 0)
		{
			perror("[server]Eroare la write() catre client.\n");
			return;
		}

		if (write(tdL.cl, s->link, 100) <= 0)
		{
			perror("[server]Eroare la write() catre client.\n");
			return;
		}

		if (write(tdL.cl, &s->id, sizeof(int)) <= 0)
		{
			perror("[server]Eroare la write() catre client.\n");
			return;
		}

		lungime = strlen(comentarii);
		write(tdL.cl, &lungime, sizeof(lungime));
		if (write(tdL.cl, comentarii, lungime) <= 0)
		{
			perror("[server]Eroare la write() catre client.\n");
			return;
		}

		lungime = strlen(rating);
		write(tdL.cl, &lungime, sizeof(lungime));
		if (write(tdL.cl, rating, lungime) <= 0)
		{
			perror("[server]Eroare la write() catre client.\n");
			return;
		}
		fflush(stdout);

		while (1)
		{
			//printf("1.Voteaza cantec\n2.Comenteaza\n0.Inapoi la meniul principal\n");
			int optiune;

			if (read(tdL.cl, &optiune, sizeof(int)) <= 0)
			{
				perror("[server]Eroare la read() de la client.\n");
				return;
			}

			if (optiune == 1)
			{
				printf("[Thread %d] %s voteaza melodia : %s - %s\n", tdL.idThread,tdL.client->name,s->artist,s->name);
				float vot;
				if (read(tdL.cl, &vot, sizeof(float)) <= 0)
				{
					perror("[server]Eroare la read() de la client.\n");
					return;
				}
				char *rasp = adaugaVot(tdL, vot, s->id);
				printf("\n%s", rasp);
				if (write(tdL.cl, rasp, 50) <= 0)
				{
					perror("[server]Eroare la write() catre server.\n");
					return;
				}
			}
			else if (optiune == 2)
			{
				printf("[Thread %d] %s comenteaza melodia : %s - %s\n", tdL.idThread,tdL.client->name,s->artist,s->name);
				char comentariu[100];
				bzero(comentariu, 100);
				if (read(tdL.cl, comentariu, 100) <= 0)
				{
					perror("[server]Eroare la read() de la client.\n");
					return;
				}
				char *rasp = adaugaComentariu(tdL, comentariu, s->id);
				printf("\n%s", rasp);
				if (write(tdL.cl, rasp, 50) <= 0)
				{
					perror("[server]Eroare la write() catre server.\n");
					return;
				}
			}
			else if (optiune == 0)
				break;
		}
	}
}

void addSong_C(thData tdL)
{
	printf("[Thread %d]Adauga melodie: %s\n", tdL.idThread, tdL.client->name);
	song *melodie = (struct song *)malloc(sizeof(struct song));
	char *name, *artist, *desc, *link, *genres;
	int lungime;
	bzero(melodie, sizeof(melodie));

	//citim datele de la client:
	read(tdL.cl, &lungime, sizeof(int));
	name = malloc(lungime + 1);
	if (read(tdL.cl, name, lungime) <= 0)
	{
		perror("[server]Eroare la read() de la client.\n");
		return;
	}
	strncpy(melodie->name, name, lungime);
	melodie->name[lungime] = '\0';

	read(tdL.cl, &lungime, sizeof(int));
	artist = malloc(lungime + 1);
	if (read(tdL.cl, artist, lungime) <= 0)
	{
		perror("[server]Eroare la read() de la client.\n");
		return;
	}
	strncpy(melodie->artist, artist, lungime);
	melodie->artist[lungime] = '\0';

	read(tdL.cl, &lungime, sizeof(int));
	desc = malloc(lungime + 1);
	if (read(tdL.cl, desc, lungime) <= 0)
	{
		perror("[server]Eroare la read() de la client.\n");
		return;
	}
	strncpy(melodie->description, desc, lungime);
	melodie->description[lungime] = '\0';

	read(tdL.cl, &lungime, sizeof(int));
	link = malloc(lungime + 1);
	if (read(tdL.cl, link, lungime) <= 0)
	{
		perror("[server]Eroare la read() de la client.\n");
		return;
	}
	strncpy(melodie->link, link, lungime);
	melodie->link[lungime] = '\0';

	read(tdL.cl, &lungime, sizeof(int));
	genres = malloc(lungime + 1);
	if (read(tdL.cl, genres, lungime) <= 0)
	{
		perror("[server]Eroare la read() de la client.\n");
		return;
	}
	//printf("%d %s",lungime,genres);
	char *genres2 = malloc(lungime + 1);
	strncpy(genres2, genres, lungime);
	genres2[lungime] = '\0';
	printf("%d %s", lungime, genres2);
	//strncpy(melodie->link,genres,lungime);

	//prelucrare genuri
	//char* genre=strtok(genres,",");

	//inseram in BD melodia
	char *mesaj = addSong_BD(melodie, genres2);

	//trimitem raspuns la client
	if (write(tdL.cl, mesaj, 50) <= 0)
	{
		perror("[server]Eroare la write() catre client.\n");
		return;
	}
	else
		printf("[Thread %d] %s", tdL.idThread, mesaj);

	fflush(stdout);
	return;
}

void getTop(thData tdL)
{
	char *type = malloc(15);
	if (read(tdL.cl, type, 15) <= 0)
	{
		perror("[server]Eroare la read() de la client.\n");
		return;
	}

	if (type[strlen(type) - 1] == '\n')
		type[strlen(type) - 1] = '\0';

	printf("[Thread %d]Vizualizare top %s: %s\n", tdL.idThread, type, tdL.client->name);

	char *top = getTop_BD(type);
	int lungime = 0;
	//printf("%s", top);
	if (top != NULL)
		lungime = strlen(top);

	if (write(tdL.cl, &lungime, sizeof(int)) <= 0)
	{
		perror("[client]Eroare la write() spre client.\n");
		return;
	}

	if (write(tdL.cl, top, lungime) <= 0)
	{
		perror("[client]Eroare la write() spre client.\n");
		return;
	}
}

int sendUser(thData tdL)
{
	char name[30], password[30];
	printf("[Thread %d]Solicitare date: %s\n", tdL.idThread, tdL.client->name);
	fflush(stdout);
	strcpy(name, tdL.client->name);
	strcpy(password, tdL.client->password);

	if (write(tdL.cl, &name, 30) <= 0)
	{
		perror("[client]Eroare la write() spre client.\n");
		return -1;
	}

	if (write(tdL.cl, &password, 30) <= 0)
	{
		perror("[client]Eroare la write() spre client.\n");
		return -1;
	}

	if (write(tdL.cl, &tdL.client->vote, sizeof(int)) <= 0)
	{
		perror("[client]Eroare la write() spre client.\n");
		return -1;
	}

	if (write(tdL.cl, &tdL.client->admin, sizeof(int)) <= 0)
	{
		perror("[client]Eroare la write() spre client.\n");
		return -1;
	}
	//printf("Am trimis\n");
	return 1;
}

void welcome(thData tdL)
{
	int nr;
	while (1)
	{
		//printf("[welcome]\n");
		if (read(tdL.cl, &nr, sizeof(int)) <= 0)
		{
			printf("[Thread %d]S-a intrerupt...", tdL.idThread);
			perror("Eroare la read() de la client.");
			break;
		}
		//printf ("[Thread %d]Mesajul a fost receptionat...%d\n",tdL.idThread, nr);
		//printf("%d\n", nr);
		if (nr == 0)
		{
			printf("[Thread %d]S-a deconectat...\n", tdL.idThread);
			fflush(stdout);
			break;
		}
		else
			parseaza(tdL, nr, "welcome");
	}
	return;
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
	free(tdL.client);
}
