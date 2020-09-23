#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>

typedef struct Parse
{
    char* request;
    char* method;
    char* name;
}parse_t;

typedef struct Task
{
    int id;
    char* name;
    struct Task * nextTask;
}task_t;

int SetSocket();
parse_t* Recieve(int clientSocket, parse_t* parseRequest);
parse_t* ParseName(parse_t* parseRequest);
parse_t* ParseMethod(parse_t* parseRequest);
int MethodID(parse_t* parseRequest);
task_t* Post (parse_t* parseRequest, task_t* node);
void MethodInterractive(int socketClient, parse_t* parseRequest, task_t* head, int id);
void Get(int socketClient, task_t* head);
char* BuildResponse();
void RequestInterractive(int clientSocket, char* request, int id);

int main()
{
    task_t* head = NULL;
    parse_t* parseRequest = NULL;

    parseRequest = malloc(sizeof(parse_t));
    int socketClient = SetSocket();
    MethodInterractive(socketClient, parseRequest, head, MethodID(ParseName(ParseMethod(Recieve(socketClient, parseRequest)))));
    free(parseRequest);
}

int SetSocket()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in setConnect;
    setConnect.sin_family = AF_INET;
    setConnect.sin_port = ntohs(8080);
    setConnect.sin_addr.s_addr = inet_addr("127.0.0.1");

    bind(serverSocket, (const struct sockaddr*) &setConnect, sizeof(setConnect));
    listen(serverSocket, 1);

    int clientSocket = accept(serverSocket, NULL, NULL);

    return clientSocket;

}
parse_t* Recieve(int clientSocket, parse_t* parseRequest)
{
    char buffer[1000];
    recv(clientSocket, buffer, sizeof(buffer), 0);
    size_t size = strlen(buffer);
    char request[size];
    strcpy(request, buffer);
    parseRequest->request = request;

    //printf("Request: \n%s\n size[%ld]\n", request, size);  //show full request and size

    return parseRequest;
}

parse_t* ParseName(parse_t* parseRequest)
{
    if(strcmp(parseRequest->method, "POST") == 0)
    {
        char* openBracket = strchr(parseRequest->request, '{');
        openBracket = openBracket + 2;
        char* closeBracket = strchr(parseRequest->request, '}');
        closeBracket = closeBracket - 1;

        int i = 0;
        while(parseRequest->request[i] != openBracket[0])
        {
            i++;
        }

        char buffer[100];
        int j = 0;
        while(parseRequest->request[i] != closeBracket[0])
        {
            buffer[j] = parseRequest->request[i];
            i++;
            j++;
        }

        size_t size = strlen(buffer);
        char name[size];
        strcpy(name, buffer);
        parseRequest->name = name;
        printf("name = %s\n", name);     //show name
    }
    return parseRequest;
}

parse_t* ParseMethod(parse_t* parseRequest)
{
    char* space = strchr(parseRequest->request, ' ');

    char buffer[100];
    int i = 0;
    while(parseRequest->request[i] != space[0])
    {
        buffer[i] = parseRequest->request[i];
        i++;
    }
    
    size_t size = strlen(buffer);
    char method[size];
    strcpy(method, buffer);

    parseRequest->method = method;
    printf("method %s\n", method);   //show method

    return parseRequest;
}

int MethodID(parse_t* ParseRequest)
{
    if(strcmp(ParseRequest->method, "GET") == 0)
    {
        return 0;
    }
    if(strcmp(ParseRequest->method, "POST") == 0)
    {
        return 1;
    }
    if(strcmp(ParseRequest->method, "PUT") == 0)
    {
        return 2;
    }
    if(strcmp(ParseRequest->method, "DELETE") == 0)
    {
        return 3;
    }

    return -1;
}

task_t* Post (parse_t* parseRequest, task_t* head)
{
    if(head == NULL)
    {
        head = malloc(sizeof(task_t));
        head->id = 0;

        head->name = parseRequest->name;
        free(parseRequest);

        head->nextTask = NULL;

        return head;
    }

    static int i = 0;

    task_t* pointer = head;
    while(pointer != NULL)
    {
        pointer = pointer->nextTask;
    }

    pointer = malloc(sizeof(task_t));
    pointer->id = i++;

    pointer->name = parseRequest->name;
    free(parseRequest);

    pointer->nextTask = NULL;

    return head;
}

void Get(int socketClient, task_t* head)
{
    if(head == NULL)
    {   
        RequestInterractive(socketClient, BuildResponse(), 0);
        return;
    }


    int i = 0;
    int j = 0;

    task_t* pointer = head;
    size_t sizeOfNames = 0;
    while(pointer != NULL)
    {
       sizeOfNames = sizeOfNames + strlen(pointer->name);
    }
    printf("sizeofNames %ld", sizeOfNames);
}

void MethodInterractive(int socketClient, parse_t* parseRequest, task_t* head, int id)
{
    switch (id)
    {
        case 0:
            Get(socketClient, head);
            break;
        case 1:
            Post(parseRequest, head);
            break;
    }
}

char* BuildResponse()
{
    char emptyTask[] = "The task is empty";
    size_t sizeOfEmptyTask = strlen(emptyTask);
    
    char status[] = "HTTP/1.1 200 OK \n";
    size_t sizeOfStatus = strlen(status);

    char type[] = "Content-Type: text \n";
    size_t sizeOfType = strlen(type);

    char length[] = "Content-Length: ";
    size_t sizeOfLength = strlen(length);

    char lineBreak[] = "\n";
    size_t sizeOflineBreak = strlen(lineBreak);

    char* charSizeEmptyTask = malloc(sizeof(char));
    sprintf(charSizeEmptyTask, "%ld", sizeOfEmptyTask);
    size_t sizeOfCharSizeEmptyTask = strlen(charSizeEmptyTask);

    char fullContentLength[sizeOfLength + sizeOfCharSizeEmptyTask];
    strcat(length, charSizeEmptyTask);
    size_t sizeOfFullContentLength = strlen(fullContentLength);
    
    size_t fullSizeOfRequest = sizeOfStatus + sizeOfType + sizeOfFullContentLength + sizeOflineBreak + sizeOfEmptyTask;
    char* response = malloc(fullSizeOfRequest);

    strcat(response, status);
    strcat(response, type);
    strcat(response, length);
    strcat(response, lineBreak);
    strcat(response, lineBreak);
    strcat(response, emptyTask);

    printf("%s \n", response);
    return response;
}

void RequestInterractive(int clientSocket, char* response, int id)
{
    switch (id)
    {
    case 0:
        send(clientSocket, response, sizeof(response), 0);
        break;
    
    default:
        break;
    }
}

