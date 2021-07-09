# TOP MUSIC
   Proiect pentru "Retele de Calculatoare", anul 2, Facultatea de Informatica IASI.

 Proiectul consta intr-o aplicatie de manageriere a unui top muzical, utilizand conceptele invatate la cursul de Retele de Calculatoare. (threaduri, concurentialitate, comunicare intre server si client, servire concurenta a threadurilor care se conecteaza la server)


## FIRST RUN:
####1. create the DB environment
Open a new terminal and execute an update :
       >sudo apt-get update
Install mysql on your computer. After running the first command you will be required to choose the password of the root user in mysql:
      >sudo apt-get install mysql-server			//installing mysql
      >sudo service mysql start						//starting the database
      >mysql -u root -p									//login with root user
	  >Enter password: <root_password> 	//write here the password you chose when installing the database, at command #2)
After this step the mysql terminal(mysql>) should be ready to use.

Create a new database:
         
		 mysql>show databases;
         mysql>create database <my_database>;

Create all the tables using the "database_tables" file:
         
		 mysql>use <my_database>
 Now you can create the tables for the project. Copy and paste in the mysql terminal all the "Create Table"-statements in "database_tables" file
         
		 mysql> create table...
####2. create in the main directory a file "data.txt"
In this file write the following pairs :
    
	password=<root_password>
    database=<my_database>
 For example: 
 
	password=thepassword123
    database=project_database

####3. compile in terminal the client and server using the command : ./cr.sh
####4. run:

	./serv.o		//to start the server
    ./cli.o			//to start the client

## RUN THE PROJECT:
1. start mysql database:

    	sudo service mysql start
2. run:

		./serv.o		//to start the server
    	./cli.o			//to start the client
		
## USING THE DATABASE:
To make sure that the database is running, execute in terminal the command:
		
		sudo service mysql start;
Login with root user:
		
		mysql -u root -p
Use the project's database:

		mysql> use <my_database>;
Now you can create tables, insert data etc.

Let's see what tables we have in the current database:

		mysql>show tables;

If you want to see what are the columns of a table, use:

		mysql>describe <table_name>;

For example:

		mysql>describe Songs;
		+-------------+--------------+------+-----+--------------------------+----------------+
		| Field       | Type         | Null | Key | Default                  | Extra          |
		+-------------+--------------+------+-----+--------------------------+----------------+
		| id          | int          | NO   | PRI | NULL                     | auto_increment |
		| name        | varchar(50)  | YES  |     | NULL                     |                |
		| artist      | varchar(50)  | NO   |     | NULL                     |                |
		| link        | varchar(100) | NO   |     | NULL                     |                |
		| description | varchar(250) | YES  |     | No description available |                |
		+-------------+--------------+------+-----+--------------------------+----------------+
		5 rows in set (0.07 sec)


