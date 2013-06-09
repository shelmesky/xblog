#include <string.h>

#define MAX_HEAD_SIZE 4096

static char * HTTP_VERSION = "HTTP/1.1";
static char * SERVER_NAME_FILED = "Server: ";
static char * SERVER_NAME = "Xblog Server 1.0";
static char * CONTENT_TYPE_FILED = "Content-Type: ";
static char * DATE_FILED = "Date: ";
static char * CONTENT_LENGTH_FIELD = "Content-Length: ";

typedef struct{
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


typedef struct {
    char * status;
    char * content_type;
    char * date;
    char * content_length;
}response_header_t;


typedef struct {
    char * raw;
    int length;
}response_content_t;


request_header_t * parse_request(char * buffer);
response_content_t * make_response(response_header_t * resp_header);
