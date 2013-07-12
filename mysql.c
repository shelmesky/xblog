#include "common.h"

int connect_mysql(MYSQL * mysql, const char * host, const char * username,
				  const char * password, const char * db)
{
	char * utf8_query;
	int conn_result=1;

	utf8_query = "SET NAMES utf8";
	
	mysql_init(mysql);

	if(!mysql_real_connect(mysql, host, username, password, db, 0, NULL, 0)) {
		fprintf(stderr, "Error connect to server: %s\n", mysql_error(mysql));
	}
	else {
		conn_result = 0;
		fprintf(stderr, "Connected to mysql server.\n");
	}

	//mysql_real_query(mysql, utf8_query, (unsigned int)strlen(utf8_query));

	return conn_result;
}

MYSQL_RES * query_mysql(MYSQL * mysql, const char * query){
	int t;
	MYSQL_RES * result;
	
	unsigned int query_length;
	query_length = (unsigned int)strlen(query);

	t = mysql_real_query(mysql, query, query_length);
	if(t) {
		fprintf(stderr, "cannot execute query: %s\n", mysql_error(mysql));
	}
	else {
		fprintf(stderr, "[%s] make...", query);
	}

	result = mysql_store_result(mysql);
	return result;
}

