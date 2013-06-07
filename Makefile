all:
	gcc -O2 -o server.run server.c

debug:
	gcc -o server.run server.c -g
