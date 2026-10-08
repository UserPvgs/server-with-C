#include <winsock2.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include "server.h"
#include "threads.h"
#define BUFFER_SIZE 1024

DWORD WINAPI receiveBroadcastThread(LPVOID data){
    ThreadClientParameter* arg = (ThreadClientParameter*)data;
    while (1)
    {
        int bytesReceived = recv(arg->clientFd, arg->buffer, BUFFER_SIZE - 1, 0);
        if(bytesReceived <= 0){
            break;
        }
        arg->buffer[bytesReceived] = '\0';
        printf("\nMensagem recebida: %s\n", arg->buffer);
    }
    return 0;
}

int main(){
    char* message = malloc(BUFFER_SIZE);
    char* buffer = malloc(BUFFER_SIZE);
    ThreadClientParameter threadParameter;
    if(message == NULL || buffer == NULL)
    {
        printf("Erro ao alocar memória\n");
        return 1;
    }
    char name[120];
    struct sockaddr_in addr_in = {0};
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(8000);
    addr_in.sin_addr.s_addr = inet_addr("127.0.0.1");
    SOCKET clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if(invalidSocket(clientFd)){
        printf("Socket inválido para rede");
        return 1;
    }
    threadParameter.clientFd = clientFd;
    threadParameter.buffer = buffer;
    if(connect(clientFd, (struct sockaddr*)&addr_in, sizeof(addr_in)) == SOCKET_ERROR){
        printf("Erro ao conectar -> %d\n", WSAGetLastError());
        free(message);
        free(buffer);
        closesocket(clientFd);
        WSACleanup();
        return 1;
    }
    HANDLE threadBroadcast = CreateThread(NULL, 0, receiveBroadcastThread, &threadParameter, 0, NULL);
    if(threadBroadcast == NULL){
        printf("Error on creation of thread");
        return 1;
    }
    while(1){
        if(fgets(message, BUFFER_SIZE, stdin) == NULL){
        printf("Por favor, insira uma mensagem válida");
        break;
        }
        printf("%s\n\n", message);
        send(clientFd, message, strlen(message), 0);
        /*int bytesReceived = recv(clientFd, buffer, BUFFER_SIZE - 1, 0);
        if(bytesReceived <= 0){
            printf("Error: invalid or empty message returned");
            break;
        }
        buffer[bytesReceived] = '\0';
        printf("\nRESPOSTA RECEBIDA:\n");
        printf("%s\n", buffer);*/
    }
    free(message);
    free(buffer);
    closesocket(clientFd);
    WSACleanup();
}