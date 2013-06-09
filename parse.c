#include "parse.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


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