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
        if(clients[i].loggedIn == true){
            printf("Broadcast para cliente %d: %s",(int)clients[i].clientSocket,message);
            send(clients[i].clientSocket, message, strlen(message), 0);
        }
    }
}

void accessChat(char buffer[8192], SOCKET clientFd, Client* client)
{
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        if(client[i].connected && client[i].clientSocket == clientFd)
        {
            char* argument = strchr(buffer, ' ');
            if(argument == NULL || argument[1] == '\0')
            {
                const char* errorMessage =
                    "Invalid LOGIN\n";

                send(
                    clientFd,
                    errorMessage,
                    strlen(errorMessage),
                    0
                );

                return;
            }
            argument++;
            strncpy(
                client[i].name,
                argument,
                sizeof(client[i].name) - 1
            );
            client[i].name[
                sizeof(client[i].name) - 1
            ] = '\0';
            client[i].loggedIn = true;
            const char* response =
                "LOGIN OK\n";
            send(
                clientFd,
                response,
                strlen(response),
                0
            );
            return;
        }
    }
    const char* errorMessage = "Socket not found\n";
    send(
        clientFd,
        errorMessage,
        strlen(errorMessage),
        0
    );
}

Client* findClient(Client* client, SOCKET clientFd)
{
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(client[i].connected == true && client[i].clientSocket == clientFd){
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
        specificClient->connected = false;
        specificClient->loggedIn = false;
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

int receiveConnection(SOCKET serverFd, struct sockaddr_in address, Client* client, Message* messageSender, fd_set readFds)
{
     char buffer[8192];

    while(1)
    {
        int addressLength = sizeof(address);
        FD_ZERO(&readFds);
        FD_SET(serverFd, &readFds);
        for(int i = 0; i < MAX_CLIENTS; i++)
        {
            if(client[i].connected)
            {
                FD_SET(client[i].clientSocket, &readFds);
            }
        }
        int resultSelect = select(
            0,
            &readFds,
            NULL,
            NULL,
            NULL
        );

        if(resultSelect == SOCKET_ERROR)
        {
            printf(
                "Erro no select -> %d\n",
                WSAGetLastError()
            );

            break;
        }
        if(FD_ISSET(serverFd, &readFds))
        {
            SOCKET clientFd = accept(
                serverFd,
                (struct sockaddr*)&address,
                &addressLength
            );

            if(clientFd == INVALID_SOCKET)
            {
                printf(
                    "Erro no accept -> %d\n",
                    WSAGetLastError()
                );

                continue;
            }
            bool inserted = false;
            for(int i = 0; i < MAX_CLIENTS; i++)
            {
                if(client[i].connected == false)
                {
                    client[i].clientSocket = clientFd;
                    client[i].connected = true;
                    client[i].loggedIn = false;
                    client[i].name[0] = '\0';

                    inserted = true;

                    printf(
                        "Novo socket conectado: %llu\n",
                        (unsigned long long)clientFd
                    );

                    break;
                }
            }
            if(inserted == false)
            {
                const char* errorMessage =
                    "Server full\n";

                send(
                    clientFd,
                    errorMessage,
                    strlen(errorMessage),
                    0
                );

                closesocket(clientFd);
            }
        }
        for(int i = 0; i < MAX_CLIENTS; i++)
        {
            if(
                client[i].connected &&
                FD_ISSET(
                    client[i].clientSocket,
                    &readFds
                )
            )
            {
                int bytesReceived = recv(
                    client[i].clientSocket,
                    buffer,
                    sizeof(buffer) - 1,
                    0
                );
                if(bytesReceived <= 0)
                {
                    closesocket(client[i].clientSocket);

                    client[i].clientSocket = INVALID_SOCKET;
                    client[i].connected = false;
                    client[i].loggedIn = false;
                    client[i].name[0] = '\0';

                    printf(
                        "Cliente desconectado\n"
                    );

                    continue;
                }

                buffer[bytesReceived] = '\0';

                printf(
                    "Recebido: %s\n",
                    buffer
                );

                parseCommand(
                    buffer,
                    client[i].clientSocket,
                    client,
                    messageSender
                );
            }
        }
    }
    return 0;
}

int main()
{
    Client* client = malloc(sizeof(Client) * MAX_CLIENTS);
    Message* messageSender = malloc(sizeof(Message) * MAX_MESSAGES);
    fd_set readFds;
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
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        client[i].clientSocket = INVALID_SOCKET;
        client[i].connected = false;
        client[i].loggedIn = false;
        client[i].name[0] = '\0';
    }
    
    receiveConnection(serverFd, address, client, messageSender, readFds);
    free(client);
    free(messageSender);
    closesocket(serverFd);
    WSACleanup();

    return 0;
}