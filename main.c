#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <stdbool.h>

#pragma comment(lib, "ws2_32.lib")

const int PORT = 8000;

typedef struct
{
    char method[16];
    char path[256];

    char host[256];
} HttpRequest;

bool invalidSocket(SOCKET serverFd);
bool startListen(SOCKET serverFd, int backlog);
int bindSocketFn(SOCKET serverFd, struct sockaddr_in address);

HttpRequest parseRequest(char* buffer);

const char* homeResponse();
const char* usersResponse();
const char* notFoundResponse();

const char* routeRequest(HttpRequest request);

int main()
{
    struct sockaddr_in address = {0};
    WSADATA wsaData;

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    SOCKET serverFd = socket(AF_INET, SOCK_STREAM, 0);

    if (invalidSocket(serverFd))
    {
        return 1;
    }

    if (bindSocketFn(serverFd, address) != 0)
    {
        printf("Erro no bind -> %d\n", WSAGetLastError());
        return 1;
    }

    if (!startListen(serverFd, 10))
    {
        return 1;
    }

    printf("Servidor iniciado na porta %d\n", PORT);

    while (1)
    {
        int addressLength = sizeof(address);

        SOCKET clientFd = accept(
            serverFd,
            (struct sockaddr*)&address,
            &addressLength
        );

        if (clientFd == INVALID_SOCKET)
        {
            printf(
                "Erro no accept -> %d\n",
                WSAGetLastError()
            );
            continue;
        }

        printf("\nCliente conectado!\n");

        char buffer[8192];

        int bytesReceived = recv(
            clientFd,
            buffer,
            sizeof(buffer) - 1,
            0
        );

        if (bytesReceived <= 0)
        {
            closesocket(clientFd);
            continue;
        }

        buffer[bytesReceived] = '\0';

        printf("\nREQUISIÇÃO RECEBIDA:\n");
        printf("%s\n", buffer);

        HttpRequest request = parseRequest(buffer);

        printf("\nMétodo: %s\n", request.method);
        printf("Path: %s\n", request.path);

        const char* response =
            routeRequest(request);

        send(
            clientFd,
            response,
            strlen(response),
            0
        );

        closesocket(clientFd);
    }

    closesocket(serverFd);
    WSACleanup();

    return 0;
}

bool invalidSocket(SOCKET serverFd)
{
    if (serverFd == INVALID_SOCKET)
    {
        printf(
            "Erro ao criar socket -> %d\n",
            WSAGetLastError()
        );

        return true;
    }

    return false;
}

int bindSocketFn(
    SOCKET serverFd,
    struct sockaddr_in address
)
{
    return bind(
        serverFd,
        (struct sockaddr*)&address,
        sizeof(address)
    );
}

bool startListen(
    SOCKET serverFd,
    int backlog
)
{
    if (listen(serverFd, backlog) != 0)
    {
        printf(
            "Erro listen -> %d\n",
            WSAGetLastError()
        );

        return false;
    }

    return true;
}

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

    if (
        strcmp(request.method, "GET") == 0 &&
        strcmp(request.path, "/usuarios") == 0
    )
    {
        return usersResponse();
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

const char* usersResponse()
{
    return
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
        "<h1>Usuarios</h1>";
}

const char* notFoundResponse()
{
    return
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "\r\n"
        "<h1>404 - Pagina nao encontrada</h1>";
}