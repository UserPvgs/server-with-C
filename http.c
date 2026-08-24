#include "http.h"
HttpRequest parseRequest(char* buffer)
{
    HttpRequest request;

    memset(&request, 0, sizeof(request));

    sscanf(
        buffer,
        "%15s %255s",
        request.method,
        request.path
    );

    return request;
}
const char* routeRequest(HttpRequest request)
{
    if (
        strcmp(request.method, "GET") == 0 &&
        strcmp(request.path, "/") == 0
    )
    {
        return homeResponse();
    }

    return notFoundResponse();
}

const char* homeResponse()
{
    return
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
        "<h1>Home</h1>";
}

const char* notFoundResponse()
{
    return
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
        "<h1>404 - Pagina nao encontrada</h1>";
}