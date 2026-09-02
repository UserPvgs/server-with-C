#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <stdbool.h>
#include "server.h"
#include "http.h"
#include "client.h"
#define MAX_CLIENTS 4
#define MAX_MESSAGES 500

const int PORT = 8000;

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

void broadcast(Client* clients, SOCKET sender, const char* message)
{
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i].connect == true){
            send(clients[i].clientSocket, message, strlen(message), 0);
        }
    }
}

void accessChat(char buffer[8192], SOCKET clientFd, Client* client)
{
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(client[i].connect == false){
            client[i].clientSocket = clientFd;
            client[i].connect = true;
            char* argument = strchr(buffer, ' ') + 1;
            strncpy(client[i].name,argument,sizeof(client[i].name) - 1);
            client[i].name[sizeof(client[i].name) - 1] = '\0';
            return;
        }
    }
    const char* errorMessage = "no more space to socket access login.";
    send(clientFd, errorMessage, strlen(errorMessage), 0);
}

Client* findClient(Client* client, SOCKET clientFd)
{
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(client[i].connect == true && client[i].clientSocket == clientFd){
            return &client[i];
        }
    }
    return NULL;
}

CommandsAndArguments parseCommand(char buffer[8192], SOCKET clientFd, Client* client, Message* messageSender)
{
    Commands command = CommandOfString(buffer);
    switch (command)
    {
    case LOGIN:
    {
            accessChat(buffer, clientFd, client);
        break;
    }
    case MESSAGE:
    {
        char* argument = strchr(buffer, ' ');
        if(argument == NULL || argument[1] == '\0'){
            argument = "Message value is empty";
            send(clientFd, argument, strlen(argument), 0);
            break;
        }
        argument++;
        //char* message = malloc(strlen(argument) + 1);
        //strcpy(message, argument);
        broadcast(client, clientFd, argument);
        Client* sender = findClient(client, clientFd);
        if(sender == NULL || sender->name[0] == '\0'){
            const char* errorMessage = "The sender not registered";
            send(clientFd, errorMessage, strlen(errorMessage), 0);
            break;
        }
        for(int i = 0; i < MAX_MESSAGES; i++){
            if(messageSender[i].content[0] == '\0'){
                strcpy(messageSender[i].content, argument);
                strcpy(messageSender[i].sender, sender->name);
                break;
            }
        }
        break;
    }
    case BYE:
        Client* specificClient = findClient(client, clientFd);
        if(specificClient == NULL){
            const char* errorMessage = "You are not logged in.";
            send(clientFd, errorMessage, strlen(errorMessage), 0);
            break;
        }
        closesocket(specificClient->clientSocket);
        specificClient->clientSocket = INVALID_SOCKET;
        specificClient->connect = false;
        specificClient->name[0] = '\0';
        break;
    case LIST:
        char listMessage[623]; 
        for(int i = 0; i < MAX_MESSAGES; i++){
            if(messageSender[i].sender == NULL || messageSender[i].sender[0] == '\0' || messageSender[i].content == NULL || messageSender[i].content[0] == '\0'){
                continue;
            }
            snprintf(listMessage, sizeof(listMessage), "%s: %s \n", messageSender[i].sender, messageSender[i].content);
            send(clientFd, listMessage, strlen(listMessage), 0);
        }
        break;
    case UNKNOWN:
        const char* message = "Command not found, try other command of list.";
        send(clientFd, message, strlen(message), 0);
        break;
    default:
        break;
    }
}

int receiveConnection(SOCKET serverFd, struct sockaddr_in address, Client* client, Message* messageSender)
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
        CommandsAndArguments commandsAndArguments = parseCommand(buffer, clientFd, client, messageSender);
    }
}

int main()
{
    Client* client = malloc(sizeof(Client) * MAX_CLIENTS);
    Message* messageSender = malloc(sizeof(Message) * MAX_MESSAGES);
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

    receiveConnection(serverFd, address, client, messageSender);
    free(client);
    free(messageSender);
    closesocket(serverFd);
    WSACleanup();

    return 0;
}