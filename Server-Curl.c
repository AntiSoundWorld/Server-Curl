#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>

void SetSocket();
void Interrection(int id);
void ClientInterrection(int clientSocket, int  id);

int main()
{
    while(true)
    {
        Interrection(0);
        SetSocket();
    }
}
void SetSocket()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in setConnect;
    setConnect.sin_family = AF_INET;
    setConnect.sin_port = htons(8080);
    setConnect.sin_addr.s_addr = inet_addr("127.0.0.1");

    bind(serverSocket, (struct sockaddr *)&setConnect, sizeof(setConnect));
    listen(serverSocket, 1);

    int clientSocket = accept(serverSocket, NULL, NULL);

    char clientGet[256];
    recv(clientSocket, clientGet, sizeof(clientGet), 0);

    ClientInterrection(clientSocket, 1);

}

void ClientInterrection(int clientSocket, int id)
{
char response[] = "Content-Type: text/html; charset=utf-8; Content-Length: <11>; HelloWorld";

    switch (id)
    {
        case 1:
            send(clientSocket, response, sizeof(response), 0);
            break;
        }
}
void Interrection(int id)
{
    switch (id)
    {
        case 0:
            printf("Server \n");
            break;
        
        case 1:
            printf("Error of connection \n");
            break;
    }
}
