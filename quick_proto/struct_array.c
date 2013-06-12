#include <stdio.h>
#include <stdlib.h>

typedef struct method_s {
	    char * method_name;
	    int code;
} method_t;

method_t method_arr[] = {
	    {"GET", 0},
	    {"POST", 1},
	    {"PUT", 2},
	    {"HEAD", 3},
	    {"DELETE", 4},
	    {NULL, 0}
};

method_t * arr_t = method_arr;

int main(int argc, char **argv)
{	
	while(arr_t->method_name != NULL)
	{
		printf("%s: %d\n", arr_t->method_name, arr_t->code);
		arr_t++;
	}
	return 0;
}
