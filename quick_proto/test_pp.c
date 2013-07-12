#include <stdio.h>

int main(int argc, char **argv)
{
	char *strings[] = {"aaaa", "bbbb", "cccc"};
	char ** s = strings;

	while(*++s  != NULL)
	{
		fprintf(stderr, "%s", *strings);
	}
	return 0;
}
