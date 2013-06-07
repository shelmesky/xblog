all:
	gcc -O2 -o server server.c

debug:
	$(all)
