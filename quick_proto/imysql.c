#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    MYSQL mysql;
    MYSQL_RES *res;
    MYSQL_ROW row;
    char * query;
    char * utf8_query;
    int t,r;
    
    mysql_init(&mysql);
    
    if(!mysql_real_connect(&mysql, "localhost", "root", "root", "xblog", 0, NULL, 0))
    {
        fprintf(stderr, "Error connect to server: %s\n", mysql_error(&mysql));
    }
    else
    {
        fprintf(stderr, "Connected to server.\n");
    }
    
    utf8_query = "SET NAMES utf8";
    query = "select * from posts";
    
    
    mysql_real_query(&mysql, utf8_query, (unsigned int)strlen(utf8_query));

	t = mysql_real_query(&mysql, query, (unsigned int)strlen(query));

	if(t){
		fprintf(stderr, "Cannot execute query: %s\n", mysql_error(&mysql));
	}
	else{
        fprintf(stderr, "[%s] make ...\n", query);
    }
    
    char * line;
	int num_fields;
	int i;

    res = mysql_store_result(&mysql);
    num_fields = mysql_num_fields(res);

	while((row = mysql_fetch_row(res))) {
		fprintf(stderr, "%s,%s,%s,%s,%s\n", row[0], row[1], row[2], row[3], row[4]);
	}
     
/*
     while(row = mysql_fetch_row(res))
     {
         for(t=0; t<mysql_num_fields(res); t++)
         {
             fprintf(stderr, "%s,", row[t]);
         }
         fprintf(stderr, "\n");
     }
*/
    
    fprintf(stderr, "mysql_free_result...\n");
    mysql_free_result(res);
    
    return 0;

}
