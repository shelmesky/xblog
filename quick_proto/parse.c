#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


const static char * req_host = "Host";
const static char * req_connection = "Connection";
const static char * req_accept = "Accept";
const static char * req_user_agent = "User-Agent";
const static char * req_accept_encoding = "Accept-Encoding";
const static char * req_accept_language = "Accept-Language";
const static char * req_accept_charset = "Accept-Charset";
const static char * req_cookie = "Cookie";
const static char * req_content_length = "Content-Length";
const static char * req_content_type = "Content-Type";


typedef struct request_header_s{
    char * host;
    char * connecton;
    char * accept;
    char * user_agent;
    char * accept_encoding;
    char * accept_charset;
    char * cookie;
    char * content_length;
    char * content_type;
    char * content;
}request_header_t;


typedef struct header_s {
	char *method;
	char *uri;
	char *version;
}header_t;


struct http_request {
    header_t * req_header;
    request_header_t * req_body;
};


typedef struct keyval_s{
	char *key;
	char *value;
	struct keyval_s * priv;
	struct keyval_s * next;
} keyval_t;


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


void list_init(keyval_t ** kv_root) {
	*kv_root = (keyval_t *)malloc(sizeof(keyval_t));
	(*kv_root)->key = NULL;
	(*kv_root)->value = NULL;
}


void parse_body(char * buf, request_header_t * req_header)
{
	char * line_deli = "\r\n";
	char * c_deli = ":";
	int in=0;
	char *p[32];
	char *outer_ptr = NULL;
	char *inner_ptr = NULL;
    char * key, * value;
	while((p[in] = strtok_r(buf, line_deli, &outer_ptr)) != NULL) {
		buf = p[in];
		p[in] = strtok_r(buf, c_deli, &inner_ptr);
        if(p[in]){
            key = ltrim(p[in]);
        }
		in++;
		buf = NULL;
        
		p[in] = strtok_r(buf, c_deli, &inner_ptr);
        if(p[in]){
            value = ltrim(p[in]);
        }
		in++;
		buf = NULL;

		p[in++] = "***";
		buf = NULL;

		//insert into link list
		char * sep;
        int ret;
        if((ret=memcmp(key, req_host, strlen(key))) == 0){
            req_header->host = value;
        }
        if((ret=memcmp(key, req_connection, strlen(key))) == 0){
            req_header->connecton = value;
        }
        if((ret=memcmp(key, req_accept, strlen(key))) == 0){
            req_header->accept = value;
        }
        if((sep=strstr(key, req_user_agent)) != NULL){
            req_header->user_agent = value;
        }
        if((sep=strstr(key, req_accept_encoding)) != NULL){
            req_header->accept_encoding = value;
        }
        if((sep=strstr(key, req_accept_charset)) != NULL){
            req_header->accept_charset = value;
        }
        if((sep=strstr(key, req_cookie)) != NULL){
            req_header->cookie = value;
        }
        if((sep=strstr(key, req_content_length)) != NULL){
            req_header->content_length = value;
        }
        if((sep=strstr(key, req_content_type)) != NULL){
            req_header->content_type = value;
        }
	}
}


struct http_request  * parse(char * buf)
{
    // copy char * to array, cause strtok only accept array
    int buf_len;
    buf_len = (int)strlen(buf);
    char buffer[buf_len];
    strncpy(buffer, buf, buf_len);
    
	//split request head and body
	char request_header[1024];
	char request_body[4096];
	split_header_body(buffer, request_header, request_body);

    //parse http request head
	header_t * req_header;
	req_header = parse_header(request_header);

    //parse http request body
    request_header_t * req_body = (request_header_t *)malloc(sizeof(request_header_t));
	parse_body(request_body, req_body);

	struct http_request * request = (struct http_request *)malloc(sizeof(struct http_request *));
    request->req_header = req_header;
    request->req_body = req_body;
    return request;
}


int main(int argc, char **argv)
{
    char * buf = "GET /entry?value=123&key=456 HTTP/1.1\r\n\
    Host: 127.0.0.1:1234\r\n\
    Connection: keep-alive\r\n\
    Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n\
    User-Agent: Mozilla/5.0 (X11; Linux x86_64) Ubuntu Chromium/25.0.1364.160 Chrome/25.0.1364.160 Safari/537.22\r\n\
    Accept-Encoding: gzip,deflate,sdch\r\n\
    Accept-Language: zh-CN,zh;q=0.8\r\n\
    Accept-Charset: UTF-8,*;q=0.5\r\n\
    Cookie: user=bWF4|1370072999|cac0893ec14c85d193159777e091b0b3eb52cd9b\r\n\r\n";
    
    struct http_request * req = (struct http_request *)malloc(sizeof(struct http_request *));
    req = parse(buf);
    
    fprintf(stderr, "uri->%s\nmethod->%s\nversion->%s\n", req->req_header->uri, req->req_header->method, req->req_header->version);
    fprintf(stderr, "host->%s\nconnection->%s\naccept->%s\n", req->req_body->host, req->req_body->connecton, req->req_body->accept);
    fprintf(stderr, "user_agent->%s\naccept_encoding->%s\naccept_charset->%s\n", req->req_body->user_agent, req->req_body->accept_encoding, req->req_body->accept_charset);
    fprintf(stderr, "cookie->%s\n", req->req_body->cookie);
    
    return 0;
}

