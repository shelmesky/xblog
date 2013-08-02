#include "common.h"

#define MAX_HEAD_SIZE 4096
#define MAX_EVENTS 10240
#define BUFSIZE 4096
#define HTTP_1_1_SUPPORT 0

static const char * LF = "\r\n";
static const char * CRLF = "\r\n\r\n";

char * HTTP_1_0_VERSION = "HTTP/1.0 ";
char * HTTP_1_1_VERSION = "HTTP/1.1 ";
char * SERVER_NAME_FILED = "Server: ";
char * SERVER_NAME = "Xblog Server 1.0";
char * CONTENT_TYPE_FILED = "Content-Type: ";
char * DATE_FILED = "Date: ";
char * CONTENT_LENGTH_FIELD = "Content-Length: ";
char * CONNECTION_FIELD = "Connection: ";

struct epoll_event ev, events[MAX_EVENTS];
struct sockaddr_in server_addr;
int listen_sock, conn_sock, nfds, epoll_fd;

MYSQL mysql;
MYSQL_RES * mysql_res = NULL;


static void add_LF(char * buf){
    strcat(buf, LF);
}


static void add_CRLF(char * buf){
    strcat(buf, CRLF);
}


struct http_request * parse_request(char * buffer){
    //fprintf(stderr, "\n%s\n", buffer);
    struct http_request * request = (struct http_request *)calloc(1, sizeof(struct http_request));
    request = parse(buffer);
    return request;
}


response_content_t * make_response(response_header_t * resp_header){
    char * resp = (char *)calloc(MAX_HEAD_SIZE, sizeof(char));
    response_content_t * resp_content = (response_content_t *)calloc(1, sizeof(response_content_t));
    char * content_length_str = (char *)calloc(1, sizeof(char));
    
    strcat(resp, HTTP_1_1_VERSION);
    strcat(resp, resp_header->status);
    add_LF(resp);
    
    strcat(resp, SERVER_NAME_FILED);
    strcat(resp, SERVER_NAME);
    add_LF(resp);
    
    strcat(resp, CONTENT_TYPE_FILED);
    strcat(resp, resp_header->content_type);
    add_LF(resp);
    
    strcat(resp, CONTENT_LENGTH_FIELD);
    strcat(resp, resp_header->content_length);
    add_LF(resp);
    
    //check if has set connection header
    if(resp_header->connecton){
        strcat(resp, CONNECTION_FIELD);
        strcat(resp, resp_header->connecton);
        add_CRLF(resp);
    }
    else {
        add_CRLF(resp);
    }
    
    resp_content->raw = resp;
    resp_content->length = strlen(resp);
    return resp_content;
}


void exit_hook(int number){
    close(listen_sock);
    fprintf(stderr, "[%d]will shutdown...[%d]\n", getpid(), number);
}


static struct io_data_t * alloc_io_data(int client_fd, struct sockaddr_in *client_addr)
{
    struct io_data_t * io_data_ptr = (struct io_data_t *)calloc(1, sizeof(struct io_data_t));
    io_data_ptr->fd = client_fd;
    io_data_ptr->in_buf = (char *)calloc(BUFSIZE, sizeof(char));
    io_data_ptr->out_buf = (char *)calloc(BUFSIZE, sizeof(char));
    io_data_ptr->in_buf_cur = 0;
    io_data_ptr->out_buf_cur = 0;
    io_data_ptr->keep_alive = 1;
    if(client_addr)
        io_data_ptr->addr = *client_addr;
    return io_data_ptr;
}


static void set_nonblocking(int fd){
    int opts;
    opts = fcntl(fd, F_GETFL);
    if(opts < 0) {
        fprintf(stderr, "fcntl failed!\n");
        return;
    }
    
    opts = opts | O_NONBLOCK;
    if(fcntl(fd, F_SETFL, opts) < 0) {
        fprintf(stderr, "fcntl failed!\n");
        return;
    }
}


static int create_sock(int port)
{
    int on = 1;
    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == -1)
    {
        perror("create socket failed!");
    }
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    setsockopt(listen_sock, IPPROTO_TCP, TCP_NODELAY, (int[]) {1}, sizeof(int));
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((short) port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if(bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind failed!");
        exit(-1);
    }
    
    if(listen(listen_sock, 1024)) {
        perror("listen failed!");
        exit(-1);
    }
    
    set_nonblocking(listen_sock);
    
    return listen_sock;
}


struct io_data_t * handle_index(struct io_data_t * client_data_ptr){
    
    client_data_ptr->out_buf_cur = 0;
    struct http_request * req = (struct http_request *)client_data_ptr->ptr;
    
    if(!HTTP_1_1_SUPPORT) {
        char * c = "handle_index: ";
        char * uri = req->req_header->uri;
        char * body = (char *)calloc(strlen(c) + strlen(uri) + 1, sizeof(char));
        strncat(body, c, strlen(c));
        strncat(body, uri, strlen(uri));
        //fprintf(stderr, "\n%s\n", body);
        
        response_header_t * resp_header = (response_header_t *)calloc(1, sizeof(response_header_t));
        char * content_length = (char *)calloc(16, sizeof(char));
        sprintf(content_length, "%d", (int)strlen(body));
        resp_header->content_length = content_length;
        resp_header->status = "200 ok";
        resp_header->content_type = "text/html";
        if(req->req_body->connecton) {
            resp_header->connecton = req->req_body->connecton;
    }
    response_content_t * resp_content = make_response(resp_header);

    strncat(client_data_ptr->out_buf, resp_content->raw, resp_content->length);
    client_data_ptr->out_buf_cur += resp_content->length;
    
    strncat(client_data_ptr->out_buf, body, strlen(body));
    client_data_ptr->out_buf_cur += strlen(body);
    
    return client_data_ptr;
    }
}


urlmap_t urlmap[] = {
    {"/", handle_index},
    {NULL, NULL}
};


static struct io_data_t * send_error(struct io_data_t * client_data_ptr, int err_code)
{
    client_data_ptr->out_buf_cur = 0;
    struct http_request * req = (struct http_request *)client_data_ptr->ptr;
    
    if(!HTTP_1_1_SUPPORT) {
    char * body = "not found";
    
    response_header_t * resp_header = (response_header_t *)calloc(1, sizeof(response_header_t));
    char * content_length = (char *)calloc(16, sizeof(char));
    sprintf(content_length, "%d", (int)strlen(body));
    resp_header->content_length = content_length;
    resp_header->status = "404 not_found";
    resp_header->content_type = "text/html";
    if(req->req_body->connecton) {
        resp_header->connecton = req->req_body->connecton;
    }
    response_content_t * resp_content = make_response(resp_header);

    strncat(client_data_ptr->out_buf, resp_content->raw, resp_content->length);
    client_data_ptr->out_buf_cur += resp_content->length;
    
    strncat(client_data_ptr->out_buf, body, strlen(body));
    client_data_ptr->out_buf_cur += strlen(body);
    
    return client_data_ptr;
    }
}


static struct io_data_t * process_request(struct io_data_t * client_data_ptr){
    
    struct http_request * req = (struct http_request *)client_data_ptr->ptr;
    
    int ret;
    urlmap_t * urlmap_p;
    urlmap_p = urlmap;
    while(1)
    {
        if(urlmap_p->callback != NULL) {
            if((ret=memcmp(req->req_header->uri, urlmap_p->url, strlen(req->req_header->uri))) == 0)
            {
                return urlmap_p->callback(client_data_ptr);
            }
            urlmap_p++;
        }
        else{
            return send_error(client_data_ptr, 404);
        }
    }
}


static void handle_read(int client_fd, struct io_data_t * client_data_ptr){
    //fprintf(stderr, "handle_read called!\n");
    int nread = 0;
    
    while((nread = read(client_fd, client_data_ptr->in_buf + client_data_ptr->in_buf_cur, BUFSIZE-1)) > 0) {
        client_data_ptr->in_buf_cur += nread;
    }
    
    if (nread ==0 || (nread == -1 && errno != EAGAIN)) {
        perror("read error");
        close(client_fd);
        return;
    }
    
    client_data_ptr->in_buf[client_data_ptr->in_buf_cur] = '\0';
    
    char * sep = NULL;
    int npos=0;
     if((sep=strstr(client_data_ptr->in_buf, CRLF)) != NULL){
         npos = sep - client_data_ptr->in_buf;
         char * request_content=(char *)calloc(MAX_HEAD_SIZE, sizeof(char));
         memcpy(request_content, client_data_ptr->in_buf, npos);
         client_data_ptr->in_buf_cur -= npos + (int)strlen(CRLF);
         
         struct http_request * req = parse_request(request_content);
         LOG(req, client_data_ptr->addr);
         
         client_data_ptr->ptr = (void *)req;
     }
    
    ev.data.ptr = (void *)process_request(client_data_ptr);
    //如果设置了data.ptr，events则只返回ptr，不会返回fd
    //ev.data.fd = client_fd;
    ev.events = EPOLLOUT | EPOLLET;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev) == -1){
      perror("epoll_ctl: mod");
      exit(-1);
    }
}


static void handle_write(int client_fd, struct io_data_t * client_data_ptr){
    //fprintf(stderr, "write buf length: %d\n", (int)strlen(client_data_ptr->out_buf)); 
    //fprintf(stderr, "handle_write called!\n"); 
    //fprintf(stderr, "Before write buffer size: %d\n", client_data_ptr->out_buf_cur);
    int nwrite;
    while(client_data_ptr->out_buf_cur >0){
        nwrite = write(client_fd, client_data_ptr->out_buf, client_data_ptr->out_buf_cur);
        client_data_ptr->out_buf_cur -= nwrite;
        
        if(nwrite < client_data_ptr->out_buf_cur) {
            if(nwrite == -1 && errno != EAGAIN) {
                perror("write error");
                close(client_fd);
                return;
            }
            break;
        }
    }
    
    //fprintf(stderr, "\n\n%s\n\n", client_data_ptr->out_buf);
    //fprintf(stderr, "After write buffer size: %d\n", client_data_ptr->out_buf_cur);
    //fprintf(stderr, "write %d bytes\n", nwrite);
    //fprintf(stderr, "write %d bytes: %s\n", nwrite, client_data_ptr->out_buf);
    
    if(!HTTP_1_1_SUPPORT) {
        close(client_fd);
        return;
    }
    
    ev.data.ptr = client_data_ptr;
    ev.events = EPOLLIN | EPOLLET;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev) == -1) {
        perror("epoll_ctl: mod");
        exit(-1);
    }
}


static void clear_all(int epoll_fd, int client_fd, struct io_data_t * client_data_ptr) {
    close(epoll_fd);
    close(client_fd);
}


int main(int argc, char **argv)
{   
    //if(argc == 1){
    //    printf("need more argument!\n");
    //    return 1;
    //}
    
    //int port = atoi(argv[1]);
	//
	char * mysql_host="127.0.0.1";
	char * mysql_username = "root";
	char * mysql_password = "root";
	char * mysql_db = "xblog";

    int port = 1234;
    int i, addrlen, client_fd;
    struct sockaddr_in client_addr;
    struct io_data_t *client_io_ptr;
    
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, exit_hook);
    signal(SIGKILL, exit_hook);
    signal(SIGQUIT, exit_hook);
    signal(SIGTERM, exit_hook);
    signal(SIGHUP, exit_hook);

	//connect to mysql server
	connect_mysql(&mysql, mysql_host, mysql_username, mysql_password, mysql_db);

    create_sock(port);
    
    if((epoll_fd = epoll_create(MAX_EVENTS)) < 0 ){
        perror("epoll_create failed!");
        exit(-1);
    }
    
    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        perror("epoll_ctl failed!");
        exit(-1);
    }
    
    fprintf(stderr, "Server listening on tcp %d\n", port);
    
    while(1){
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds == -1){
            perror("epoll_wait failed!");
            exit(-1);
        }
        
        for(i=0; i<nfds && nfds>0; ++i){
            /* 如果是listen socket */
        if(events[i].data.fd == listen_sock){
            //fprintf(stderr, "listen sock!\n");
                
            while((conn_sock = accept(listen_sock, (struct sockaddr *)&client_addr, (socklen_t *)&addrlen)) > 0){
                    set_nonblocking(conn_sock);
                    //fprintf(stderr, "conn_sock: %d\n", conn_sock);
                    struct io_data_t *ptr = alloc_io_data(conn_sock, &client_addr);
                    //fprintf(stderr, "ptr: %d\n", ptr->fd);
                    ev.data.ptr = ptr;
                    ev.events = EPOLLIN | EPOLLET;
                    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_sock, &ev) == -1){
                        perror("epoll_ctl add failed!");
                        exit(-1);
                    }
                    
                    if(conn_sock == -1) {
                        if(errno != EAGAIN && errno != ECONNABORTED
                            && errno != EPROTO && errno!= EINTR){
                            perror("accept failed!");
                            
                        }
                    }
                }
                continue;
            }
            
            //fprintf(stderr, "client sock!\n");
            /* 如果是client socket */
            client_io_ptr = (struct io_data_t *)events[i].data.ptr;
            //fprintf(stderr, "client_io_ptr->fd: %d\n", client_io_ptr->fd);
            if(client_io_ptr->fd <= 0) continue;
            
            if(events[i].events & EPOLLIN) {
                handle_read(client_io_ptr->fd, client_io_ptr);
                
            } else if(events[i].events & EPOLLOUT) {
                handle_write(client_io_ptr->fd, client_io_ptr);
                
            } else if(events[i].events & EPOLLERR) {
                client_fd = client_io_ptr->fd;
                clear_all(epoll_fd, client_fd, client_io_ptr);
            }
            
        }
        
    }
    
    return 0;
}

