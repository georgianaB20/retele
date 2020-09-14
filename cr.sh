gcc servTcpConcTh2.c -lmysqlclient -o serv.o -pthread

gcc cliTcpNr.c -o cli.o -pthread
