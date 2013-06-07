all:
	gcc -O2 -o server server.c

debug:
	gcc -o server server.c -g
