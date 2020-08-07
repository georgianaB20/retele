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

//http://www.kitebird.com/mysql-book/ch06-1ed.pdf bibliografie
void process_result_set (MYSQL *conn, MYSQL_RES *res_set)
{    
    if(mysql_num_rows(res_set)!=1)
        printf("Utilizatorul nu exista");
    if (mysql_errno(conn) != 0)
        perror(mysql_error(conn));
   
}


int main(int argc, char *argv[]){
    //conectare la baza de date;
    MYSQL *mysql=mysql_init(NULL);
    
    if(mysql==NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(mysql));
        exit(1);
    }
    if (mysql_real_connect(mysql, "localhost", "root", "elefant.2004", "proiect", 0, NULL, 0) == NULL) 
    {
        fprintf(stderr, "Am dat peste o eroare: %s\n", mysql_error(mysql));
        mysql_close(mysql);
        exit(1);
    }
    printf("Conectare la baza de date: [ OK ]\n");
    
    //verificam parametrii
    if (argc != 3)
    {
      printf ("Sintaxa: %s <username> <parola>\n", argv[0]);
      return -1;
    }
    
    char interogare[500]="";
    strcpy(interogare,"select * from Users where username like '");

    //"select * from Users where username like 'maria' and password like 'maria99'";
     strcat(interogare,argv[1]);
     strcat(interogare,"' and password like '");
     strcat(interogare,argv[2]);
     strcat(interogare,"'");
    
    //executam interogarea
    int status=mysql_query(mysql,interogare);
    if(status){
        printf("Could not execute statement(s)");
        mysql_close(mysql);
        exit(0);
    }
    /* process each statement result */
    do {
    /* did current statement return data? */
    MYSQL_RES *result = mysql_store_result(mysql);
    if (result)
    {
        /* yes; process rows and free the result set */
        process_result_set(mysql, result);
        printf("%s",(char*)result);
        mysql_free_result(result);
    }
    else          /* no result set or error */
    {
        if (mysql_field_count(mysql) == 0)
        {
        printf("%lld rows affected\n",
                mysql_affected_rows(mysql));
        }
        else  /* some error occurred */
        {
        printf("Could not retrieve result set\n");
        break;
        }
    }
    /* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
    if ((status = mysql_next_result(mysql)) > 0)
        printf("Could not execute statement\n");
    } while (status == 0);

    mysql_close(mysql);
    

}