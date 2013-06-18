#include <stdio.h>
#include <stdlib.h>
#include "common.h"


void LOG(struct http_request * req, struct sockaddr_in addr){
    fprintf(stderr, "[%s] %s %s\n", inet_ntoa(addr.sin_addr), req->req_header->method, req->req_header->uri);
}