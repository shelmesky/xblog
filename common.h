#include <sys/time.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <errno.h>
#include <mysql/mysql.h>
#include <ctemplate.h>


typedef struct response_header_s{
    char * status;
    char * content_type;
    char * date;
    char * content_length;
    char * connecton;
}response_header_t;


typedef struct response_content_s{
    char * raw;
    int length;
}response_content_t;
 

struct io_data_t {
    int fd;
    struct sockaddr_in addr;
    char * in_buf;
    char * out_buf;
    int in_buf_cur;
    int out_buf_cur;
    int keep_alive;
    void * ptr;
};

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


typedef struct urlmap_s{
    char * url;
    struct io_data_t * (*callback)(struct io_data_t *);
} urlmap_t;


extern void LOG(struct http_request *, struct sockaddr_in);
extern struct http_request * parse(char *);
extern int connect_mysql(MYSQL *, const char *, const char *, const char *, const char *);
extern MYSQL_RES * query_mysql(MYSQL *, const char *);
extern int run(void);

