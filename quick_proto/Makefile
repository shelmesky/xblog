
template:
	cc -c -O0 -o template.o template.c -g
	cc -O0 -o mytemplate  template.o -lctemplate -L ../ctemplate -g

mysql:
	gcc -O0 -o imysql imysql.c -g -lmysqlclient
