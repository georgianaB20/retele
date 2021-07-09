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
    - install mysql on your computer
    - create a new database
    - create all the tables using the "" file
2. create in the main directory the file "data.txt"
    In this file write the following pairs :
    - password=<your_mysql_password>
    - database=<name_of_the_database_you_created>
    for ex: password=thepassword123
            database=project_database

2. compile the files using the command : ./cr.sh
3. start the database using the command: sudo service mysql start
4. run the following commands:
    - to start the server: ./serv.o
    - to start the client: ./cli.o

HOW TO RUN THE PROJECT: start from step 3 in "FIRST RUN"
