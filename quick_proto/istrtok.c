#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char buf[] = "GET /host HTTP/1.1\r\nUser-Agent: curl 1.2.3\r\nHost: 127.0.0.1:1234\r\nX-User: user1\r\n\r\n";

typedef struct header_s {
	char *method;
	char *uri;
	char *version;
}header_t;

typedef struct keyval_s{
	char *key;
	char *value;
	struct keyval_s * priv;
	struct keyval_s * next;
} keyval_t;


void split_header_body(char * buffer, char *request_header, char * request_body)
{
	char * sep;
	int header_len, body_len, buffer_len;
	if((sep = strstr(buffer, "\r\n")) != NULL) {
		header_len = sep - buffer;
	}

	strncpy(request_header, buffer, header_len);
	request_header[header_len] = '\0';

	buffer_len = (int)strlen(buffer);
	strncpy(request_body, sep, buffer_len - header_len);
	request_body[buffer_len - header_len] = '\0';
}

header_t * parse_header(char * buf) {
	char * deli = " ";
	char * sp;
	header_t * header = (header_t *)malloc(sizeof(header_t));
	sp = strtok(buf, deli);
	header->method = sp;
	sp = strtok(NULL, deli);
	header->uri = sp;
	sp = strtok(NULL, deli);
	header->version = sp;
	return header;
}


void parse_body(char * buf, keyval_t * kv_root)
{
	keyval_t * kv = (keyval_t *)malloc(sizeof(keyval_t));
	char * line_deli = "\r\n";
	char * c_deli = ":";
	int in=0;
	char *p[32];
	char *outer_ptr = NULL;
	char *inner_ptr = NULL;
	while((p[in] = strtok_r(buf, line_deli, &outer_ptr)) != NULL) {
		buf = p[in];
		p[in] = strtok_r(buf, c_deli, &inner_ptr);
		kv->key = p[in];
		in++;
		buf = NULL;
		p[in] = strtok_r(buf, c_deli, &inner_ptr);
		kv->value = p[in];
		in++;
		buf = NULL;

		p[in++] = "***";
		buf = NULL;

		//insert into link list
		fprintf(stderr, "%s->%s\n", kv->key, kv->value);
	}
}


int main(int argc, char **argv)
{
	//split head and body
	char request_header[1024];
	char request_body[4096];
	split_header_body(buf, request_header, request_body);
	//fprintf(stderr, "\n%s\n\n%s\n", request_header, request_body);

	header_t * header;
	header = parse_header(request_header);
	//fprintf(stderr, "\nmethod:%s\nuri:%s\nversion:%s\n\n", header->method, header->uri, header->version);

	keyval_t * kv_root = (keyval_t *)malloc(sizeof(keyval_t));
	parse_body(request_body, kv_root);

	return 0;
}
