#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>

typedef struct Task
{
    int id;
    char * name;
    struct Task * nextTask;
}task_t;

int SetSocket();
char* ParseMethod(int clientSocket);
void RequestInterractive(int clientSocket, int id);
char* Recieve(int clientSocket);
int NumberOfMethods(int clientSocket);
task_t* MethodInterractive(int clientSocket, task_t* head, int id);
task_t* Create(int clientSocket, task_t* head);

int main()
{
    task_t* head = NULL;
    while(true)
    {
        int clientSocket = SetSocket();
        MethodInterractive(clientSocket, head, NumberOfMethods(clientSocket));
    }
}

int SetSocket()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in setConnect;
    setConnect.sin_family = AF_INET;
    setConnect.sin_port = htons(8080);
    setConnect.sin_addr.s_addr = inet_addr("127.0.0.1");

    bind(serverSocket, (struct sockaddr *)&setConnect, sizeof(setConnect));
    listen(serverSocket, 1);

    
    int clientSocket = accept(serverSocket, NULL, NULL);
    return clientSocket;

}
void RequestInterractive(int clientSocket, int id)
{
    char request[] = 
    "HTTP/1.1 200 OK\n"
    "Content-Type: text\n"
    "Content-Length: 1\n"
    "Connection: keep-alive\n"
    "\n"
    "Insert name\n";

    switch (id)
    {
    case 0:
    while(true)
    {
        send(clientSocket, request, sizeof(request), 0);
    }
    default:
        break;
    }
}
char* ParseMethod(int clientSocket)
{
    
    char* response = Recieve(clientSocket);
    char* slash = strchr(response, ' ');

    char buffer[25];

    for(int i = 0; response[i] != slash[0]; i++)
    {
        buffer[i] = response[i];
    }

    size_t sizeOfMethod = strlen(buffer);

    char method[sizeOfMethod];
    strcpy(method, buffer);

    char* pointerToMethod = method;
    //printf("pointerTomethod [<%s>]\n", pointerToMethod);

    return pointerToMethod;
}
char* Recieve(int clientSocket)
{
    char buffer[256];
    recv(clientSocket, buffer, sizeof(buffer), 0);
    size_t size = strlen(buffer);
    char response[size];
    strcpy(response, buffer);

    char * pointerToResponse = response;
    //printf("Recieve - %s\n", pointerToResponse);
    return pointerToResponse;
}

int NumberOfMethods(int clientSocket)
{   
    char* pointerToMethod = ParseMethod(clientSocket);
    size_t size = strlen(pointerToMethod);
    char method[size];
    for(int i = 0; i < size; i++)
    {
        method[i] = pointerToMethod[i];
    } 
    //printf("method [%s]\n", method);

    if(strcmp(method, "CREATE") == 0)
    {
        //RequestInterractive(clientSocket, 0);
        return 0;
    }
    if(strcmp(method, "READ") == 0)
    {
        return 1;
    }
    if(strcmp(method, "UPDATE") == 0)
    {
        return 2;
    }
    if(strcmp(method, "DELETE") == 0)
    {
        return 3;
    }

    return -1;
}

task_t* Create(int clientSocket, task_t* head)//
{
    if(head == NULL)
    {
        head = malloc(sizeof(task_t));
        head->id = 0;
        head->name = Recieve(clientSocket);
    }
    return head;
}

task_t* MethodInterractive(int clientSocket, task_t* head, int id)
{
    task_t* firstItem = head;
    switch (id)
    {
        case 0:
            firstItem = Create(clientSocket, head);
            printf("id-[%d] name - [%s]", firstItem->id, firstItem->name);
            break;
        default:
            break;
    }
    return firstItem;
}