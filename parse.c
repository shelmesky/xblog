#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_HEAD_SIZE 4096

char * HTTP_VERSION = "HTTP/1.1";
char * SERVER_NAME_FILED = "Server: ";
char * SERVER_NAME = "Xblog Server 1.0";
char * CONTENT_TYPE_FILED = "Content-Type: ";
char * DATE_FILED = "Date: ";
char * CONTENT_LENGTH_FIELD = "Content-Length: ";

typedef struct request_header_s{
    int method;
    char * uri;
    char * version;
        char * host;
        char * connecton;
        char * accept;
        char * user_agent;
        char * accept_encoding;
        char * accept_charset;
        char * cookie;
}request_header_t;


typedef struct response_header_s{
    char * status;
    char * content_type;
    char * date;
    char * content_length;
}response_header_t;


typedef struct response_content_s{
    char * raw;
    int length;
}response_content_t;


request_header_t * parse_request(char * buffer){
    
}


response_content_t * make_response(response_header_t * resp_header){
    char * resp = (char *)malloc(MAX_HEAD_SIZE);
    response_content_t * resp_content = (response_content_t *)malloc(sizeof(response_content_t));
    char * content_length_str = (char *)malloc(sizeof(char));
    strcat(resp, HTTP_VERSION);
    strcat(resp, resp_header->status);
    strcat(resp, strcat(SERVER_NAME_FILED, SERVER_NAME));
    strcat(resp, strcat(CONTENT_TYPE_FILED, resp_header->content_type));
    strcat(resp, strcat(CONTENT_LENGTH_FIELD, resp_header->content_length));
    resp_content->raw = resp;
    resp_content->length = strlen(resp);
    return resp_content;
}