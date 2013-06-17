all:
	gcc -O2 -o server.run parse.c server.c 

debug:
	gcc -O0 -o server.run server.c -g
