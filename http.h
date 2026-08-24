#ifndef HTTP_H
#define HTTP_H
typedef struct
{
    char method[16];
    char path[256];

    char host[256];
} HttpRequest;
HttpRequest parseRequest(char* buffer);
const char* homeResponse();
const char* notFoundResponse();
const char* routeRequest(HttpRequest request);
#endif