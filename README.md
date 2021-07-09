# TOP MUSIC
    Proiect pentru "Retele de Calculatoare", anul 2, Facultatea de Informatica IASI.

    Proiectul consta intr-o aplicatie de manageriere a unui top muzical, utilizand conceptele invatate la cursul de Retele de Calculatoare. 
    (threaduri, concurentialitate, comunicare intre server si client, servire concurenta a threadurilor care se conecteaza la server)

Useful commands:
    to access the database from command line use : mysql -u root -p (and write the password for mysql)
    to start the database from command line use: sudo service mysql start
    to stop the database from command line use: sudo service mysql stop
    to compile the files for the server and client use: ./cr.sh

FIRST RUN:
1. create the DB environment
    - install mysql on your computer: 
        >sudo apt-get update
        >sudo apt-get install mysql-server (here you will choose the password for the "root" user)
        >sudo service mysql start
        >mysql -u root -p
        >Enter password: <root_password> (write here the password you chose when installing, at command #2)
        After this step the mysql terminal(mysql>) should be ready to use.
    - create a new database:
         mysql>show databases;
         mysql>create database <my_database>;
         
    - create all the tables using the "database_tables" file:
         mysql>use <my_database>
         Now you can create the tables for the project. Copy and paste in the mysql terminal all the "Create Table"-statements in "database_tables" file
         mysql> create table...
2. create in the main directory a file "data.txt"
    In this file write the following pairs :
    password=<root_password>
    database=<my_database>
    for ex: password=thepassword123
            database=project_database

2. compile the client and server using the command : ./cr.sh
4. run:
    - to start the server: ./serv.o
    - to start the client: ./cli.o

RUN THE PROJECT:
1. start mysql:
    >sudo service mysql start
2. run:
    - to start the server: ./serv.o
    - to start the client: ./cli.o

