all:
	gcc -O2 -o server.run server.c parse.c utils.c mysql.c -lmysqlclient

debug:
	gcc -O0 -o server.run server.c parse.c utils.c mysql.c -lmysqlclient -g

install:
	install server.run /usr/local/bin
