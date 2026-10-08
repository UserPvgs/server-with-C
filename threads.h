#include <winsock2.h>
typedef struct {
    SOCKET clientFd;
    char* buffer;
} ThreadClientParameter ;