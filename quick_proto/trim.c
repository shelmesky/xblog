#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

char * ltrim(char * buffer)
{
	char * temp;
	temp = buffer;
	while(*temp && isspace(*temp)) temp++;
	return temp;
}

char * rtrim(char *buf)
{
	if(buf == NULL) return;
	char * buffer = (char *)malloc(sizeof(buf[0]) * strlen(buf));
	memcpy(buffer, buf, strlen(buf) - 1);
	char * temp = buffer;
	char * temp1;
	temp1 = temp + strlen(buffer) - 1;
	while(temp1 > temp && isspace(*temp1)) *temp1-- = '\0';
	return temp;
}

char *trim(char *str)
{
    char *p = str;
    char *p1;
    if(p)
    {
		p1 = p + strlen(str) - 1;
	    while(*p && isspace(*p)) p++;
	    while(p1 > p && isspace(*p1)) *p1-- = '\0';
	}
    return p;
}


int main(int argc, char **argv)
{
	char *s = "    fuck you   !!!    ";

	//printf("%saaa\n", trim(s));

	printf("%saaa\n", ltrim(s));
	printf("%saaa\n", rtrim(s));
	return 0;
}


