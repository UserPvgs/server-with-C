#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <stdbool.h>
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