#include <winsock2.h>
#include <stdbool.h>
#ifndef CLIENT_H
#define CLIENT_H

typedef struct
{
    SOCKET clientSocket;
    char name[120];
    bool connect;
} Client;

#endif