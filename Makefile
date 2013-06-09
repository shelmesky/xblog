all:
	gcc -O2 -o server.run server.c parse.c

debug:
	gcc -O0 -o server.run server.c parse.c -g
