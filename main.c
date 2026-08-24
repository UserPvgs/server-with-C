#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <stdbool.h>
#include "server.h"
#include "http.h"
#include "client.h"

const int PORT = 8000;

int main()
{
    Client* client = malloc(sizeof(Client) * 4);
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

    receiveConnection(serverFd, address, client);

    closesocket(serverFd);
    WSACleanup();

    return 0;
}

Commands CommandOfString(char buffer[8192])
{
    char command[32];

    sscanf(buffer, "%31s", command);

    if (strcmp(command, "LOGIN") == 0)
        return LOGIN;

    if (strcmp(command, "MESSAGE") == 0)
        return MESSAGE;

    if (strcmp(command, "BYE") == 0)
        return BYE;

    if (strcmp(command, "LIST") == 0)
        return LIST;
    return UNKNOWN;
}

void broadcast(Client* clients, Client* sender, const char* message)
{
    for(int i = 0; i < sizeof(clients); i++){
        if(clients[i].connect == true){
            send(clients[i].clientSocket, message, strlen(message), 0);
        }
    }
}

CommandsAndArguments parseCommand(char buffer[8192], SOCKET clientFd, Client* client)
{
    Commands command = CommandOfString(buffer);
    switch (command)
    {
    case LOGIN:
    {
            //char* message = (char *) malloc(strlen(strchr(buffer, ' ') + 1) + 1);
            client->clientSocket = clientFd;
            client->connect = true;
            char* argument = strchr(buffer, ' ') + 1;
            strncpy(client->name,argument,sizeof(client->name) - 1);
            client->name[sizeof(client->name) - 1] = '\0';
        break;
    }
    case MESSAGE:
    {
        char* message = (char *) malloc(strlen(strchr(buffer, ' ') + 1) + 1);
        broadcast(client, clientFd, message);
        break;
    }
    case BYE:
        free(client);
        break;
    case LIST:
        /* code */
        break;
    case UNKNOWN:
        /* code */
        break;
    default:
        break;
    }
}

int receiveConnection(SOCKET serverFd, struct sockaddr_in address, Client* client)
{
    char buffer[8192];
    while(1)
    {
        int addressLength = sizeof(address);

        SOCKET clientFd = accept(
            serverFd,
            (struct sockaddr*)&address,
            &addressLength
        );

        if (clientFd == INVALID_SOCKET)
        {
            printf("Erro no accept -> %d\n",WSAGetLastError());
            continue;
        }
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
        printf("%s\n", buffer);
        CommandsAndArguments commandsAndArguments = parseCommand(buffer, clientFd, client);
    }
}