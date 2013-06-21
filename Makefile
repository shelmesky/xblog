all:
	gcc -O2 -o server.run utils.c parse.c server.c

debug:
	gcc -O0 -o server.run server.c parse.c utils.c -g

install:
	install server.run /usr/local/bin
