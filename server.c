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


#define MAX_EVENTS 10240
#define BUFSIZE 4096

struct epoll_event ev, events[MAX_EVENTS];
struct sockaddr_in server_addr;
int listen_sock, conn_sock, nfds, epoll_fd;


struct io_data_t {
    int fd;
    struct sockaddr_in addr;
    char * in_buf;
    char * out_buf;
    int in_buf_cur;
    int out_buf_cur;
    int keep_alive;
};


static struct io_data_t * alloc_io_data(int client_fd, struct sockaddr_in *client_addr)
{
    struct io_data_t * io_data_ptr = (struct io_data_t *)malloc(sizeof(struct io_data_t));
    io_data_ptr->fd = client_fd;
    io_data_ptr->in_buf = (char *)malloc(BUFSIZE);
    io_data_ptr->out_buf = (char *)malloc(BUFSIZE);
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
        fprintf(stderr, "fcntl failed!");
        return;
    }
    
    opts = opts | O_NONBLOCK;
    if(fcntl(fd, F_SETFL, opts) < 0) {
        fprintf(stderr, "fcntl failed!");
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


static void handle_read(int client_fd, struct io_data_t * client_data_ptr){
    fprintf(stderr, "handle_read called!\n");
    int nread = 0;
    
    while((nread = read(client_fd, client_data_ptr->in_buf + client_data_ptr->in_buf_cur, BUFSIZE-1)) > 0) {
        client_data_ptr->in_buf_cur += nread;
    }
    if (nread == -1 && errno != EAGAIN) {
        perror("read error");
        close(client_fd);
        return;
    }
    
    client_data_ptr->in_buf[client_data_ptr->in_buf_cur] = '\0';
    
    fprintf(stderr, "recv %d byte!\n", client_data_ptr->in_buf_cur);
    
    // start handle request
    strncpy(client_data_ptr->out_buf, client_data_ptr->in_buf, client_data_ptr->in_buf_cur);
    client_data_ptr->out_buf_cur += client_data_ptr->in_buf_cur;
    // end handle request
    
    //fprintf(stderr, "handle_read/fd: %d\n", client_data_ptr->fd);
    
    ev.data.ptr = client_data_ptr;
    //如果设置了data.ptr，events则只返回ptr，不会返回fd
    //ev.data.fd = client_fd;
    ev.events = EPOLLOUT | EPOLLET;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev) == -1){
      perror("epoll_ctl: mod");
      exit(-1);
    }
}


static void handle_write(int client_fd, struct io_data_t * client_data_ptr){
    fprintf(stderr, "handle_write called!\n"); 
    int nwrite;
    while(client_data_ptr->out_buf_cur >0){
        nwrite = write(client_fd, client_data_ptr->out_buf, client_data_ptr->out_buf_cur);
        client_data_ptr->out_buf_cur -= nwrite;
    }
    
    ev.data.ptr = client_data_ptr;
    ev.events = EPOLLIN | EPOLLET;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_fd, &ev) == -1) {
        perror("epoll_ctl: mod");
        exit(-1);
    }
}


static void clear_all(int epoll_fd, int client_fd, struct io_data_t * client_data_ptr) {
    
}


int main(int argc, char **argv)
{   
    //if(argc == 1){
    //    printf("need more argument!\n");
    //    return 1;
    //}
    
    //int port = atoi(argv[1]);
    int port = 1234;
    int i, addrlen, client_fd;
    struct io_data_t *client_io_ptr;
    
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
	      fprintf(stderr, "listen sock!\n");
                
                while((conn_sock = accept(listen_sock, (struct sockaddr *)&server_addr, (socklen_t *)&addrlen)) > 0){
                    set_nonblocking(conn_sock);
		    fprintf(stderr, "conn_sock: %d\n", conn_sock);
                    struct io_data_t *ptr = alloc_io_data(conn_sock, (struct sockaddr_in *)NULL);
		    fprintf(stderr, "ptr: %d\n", ptr->fd);
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
            
            fprintf(stderr, "client sock!\n");
            /* 如果是client socket */
            client_io_ptr = (struct io_data_t *)events[i].data.ptr;
	    fprintf(stderr, "client_io_ptr->fd: %d\n", client_io_ptr->fd);
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

