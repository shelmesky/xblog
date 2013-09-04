all:
	clang -O2 -o server.run server.c parse.c utils.c mysql.c template.c -lmysqlclient -lctemplate -L ctemplate

debug:
	clang -O0 -o server.run server.c parse.c utils.c mysql.c template.c -lmysqlclient -lctemplate -L ctemplate -g

install:
	install server.run /usr/local/bin
