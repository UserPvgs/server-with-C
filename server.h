#ifndef SERVER_H
#define SERVER_H
#include <winsock2.h>
#include <stdbool.h>
typedef enum {
    LOGIN,
    MESSAGE,
    BYE,
    LIST,
    UNKNOWN
} Commands;
typedef struct {
    char sender[120];
    char content[512];
} Message;
typedef struct 
{
    Commands command;
    char argument[512];
} CommandsAndArguments;
bool invalidSocket(SOCKET serverFd);
bool startListen(SOCKET serverFd, int backlog);
int bindSocketFn(SOCKET serverFd, struct sockaddr_in address);
#endif