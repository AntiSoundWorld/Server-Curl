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

void SetSocket();
parse_t* Recieve(int clientSocket, parse_t* parseRequest);
parse_t* ParseMethod(parse_t* parseRequest);
char* IsolateBody(parse_t* parseRequest);
void ParseName(parse_t* parseRequest, char* body);
int MethodID(parse_t* parseRequest);
task_t* Post (int socketClient, parse_t* parseRequest, task_t* node);
task_t* MethodInterractive(int socketClient, task_t* head, parse_t* parseRequest);
void Get(int socketClient, task_t* head, parse_t* ParseRequest);
char* BuildResponse(int id);
void SendResponse(int clientSocket, char* request);

int main()
{
    SetSocket();
}

void SetSocket()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in setConnect;
    setConnect.sin_family = AF_INET;
    setConnect.sin_port = ntohs(8090);
    setConnect.sin_addr.s_addr = inet_addr("127.0.0.1");

    bind(serverSocket, (const struct sockaddr*) &setConnect, sizeof(setConnect));
    listen(serverSocket, 1);


    parse_t* parseRequest = (parse_t *)malloc(sizeof(parse_t));
    int clientSocket = accept(serverSocket, NULL, NULL);

    
    ParseName(parseRequest, IsolateBody(ParseMethod(Recieve(clientSocket, parseRequest))));
    free(parseRequest->request);
    free(parseRequest->method);
    free(parseRequest->name);
    free(parseRequest);

}
parse_t* Recieve(int clientSocket, parse_t* parseRequest)
{
    char buffer[1000] = "\0";
    recv(clientSocket, buffer, sizeof(buffer), 0);
    size_t sizeOfBuffer = strlen(buffer);
    parseRequest->request = malloc(sizeOfBuffer + 1);
    strcpy(parseRequest->request, buffer);
    //printf("Request:\n %s\n ", parseRequest->request);  //show full request and size

    return parseRequest;
}

parse_t* ParseMethod(parse_t* parseRequest)
{
    char buffer[100] = "\0";
    int i = 0;
    while(parseRequest->request[i] != ' ')
    {
        buffer[i] = parseRequest->request[i];
        i++;
    }
    size_t sizeOfBuffer = strlen(buffer);
    parseRequest->method = malloc(sizeOfBuffer + 1);
    strcpy(parseRequest->method, buffer);
    //printf("Method \n%s\n", parseRequest->method);   //show method

    return parseRequest;
}
char* IsolateBody(parse_t* parseRequest)

{
    if(strcmp(parseRequest->method, "POST") == 0)
    {
        char* request = parseRequest->request;
        int sizeOfRequest = strlen(request);

        int i = 0;
        int j = 2;
        
        while(j < sizeOfRequest)
        {
            if(request[i] == '\n' && request[j] == '\n')
            {
                break;
            }
            i++;
            j++;
        }

        int a = 0;
        char buffer[1024] = "\0";

        while(j < sizeOfRequest)
        {
            buffer[a] = request[j];
            j++;
            a++;
        }
        size_t sizeOfBuffer = strlen(buffer);
        char* body = malloc(sizeOfBuffer + 1);
        strcpy(body, buffer);

        return body;
    }
    return NULL;
}

void ParseName(parse_t* parseRequest, char* body)
{
    int i = 0;
    size_t sizeOfBody = strlen(body);
    while(i != sizeOfBody)
    {
        if(body[i] == ' ')
        {
            break;
        }
        i++;
    }

    i++;

    char buffer[1024] = "\0";
    int j = 0;
    while(i < sizeOfBody)
    {
        if(body[i] == '"')
        {
            break;
        }
        
        buffer[j] = body[i];
        i++;
        j++;
    }
    size_t sizeOfBuffer = strlen(buffer);

    parseRequest->name = malloc(sizeOfBuffer + 1);
    strcat(parseRequest->name, buffer);
    free(body);
    //printf("name:\n%s\n", parseRequest->name);
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

task_t* Post (int socketClient, parse_t* parseRequest, task_t* head)
{
    if(head == NULL)
    {
        head = (task_t*)malloc(sizeof(task_t));
        head->id = 0;

        head->name = (char*)malloc(strlen(parseRequest->name) * sizeof(char) + 1);
        head->name = parseRequest->name;
        head->nextTask = NULL;
        
        SendResponse(socketClient, BuildResponse(1));

        return head;
    }

    static int i = 0;

    task_t* pointer = head;
    while(pointer != NULL)
    {
        pointer = pointer->nextTask;
    }

    pointer = (task_t*)malloc(sizeof(task_t));
    pointer->id = i++;

    pointer->name = parseRequest->name;

    pointer->nextTask = NULL;

    return head;
}

void Get(int socketClient, task_t* head, parse_t* parseRequest)
{
    if(head == NULL)
    {   
        SendResponse(socketClient, BuildResponse(0));
        return;
    }

    task_t* pointer = head;
    size_t sizeOfNames = 0;
    while(pointer != NULL)
    {
        sizeOfNames = sizeOfNames + strlen(pointer->name);
        pointer = pointer->nextTask;
    }
    //printf("sizeOfNames %ld", sizeOfNames);
}

task_t* MethodInterractive(int socketClient, task_t* head, parse_t* parseRequest)
{
    switch (MethodID(parseRequest))
    {
        case 0:
            Get(socketClient, head, parseRequest);
            break;
        case 1:
            return Post(socketClient, parseRequest, head);
            break;
    }
    return head;
}

char* BuildResponse(int id)
{
    char bodyEmptyTask[] = "The task is empty";
    size_t sizeOfEmptyTask = strlen(bodyEmptyTask);
    
    char status[] = "HTTP/1.1 200 OK \n";
    size_t sizeOfStatus200 = strlen(status);

    char type[] = "Content-Type: text \n";
    size_t sizeOfType = strlen(type);


    char length[] = "Content-Length: ";
    size_t sizeOfLength = strlen(length);

    char connection[] = "Connection: close \n";
    size_t sizeOfConnection = strlen(connection);

    char lineBreak[] = "\n";
    size_t sizeOfLineBreak = strlen(lineBreak);


    if(id == 0)
    {
        char* charSizeEmptyTask = (char*)malloc(sizeOfEmptyTask * sizeof(char) + 1);
        sprintf(charSizeEmptyTask, "%ld", sizeOfEmptyTask);

        strcat(length, charSizeEmptyTask);
        strcat(length, lineBreak);
        
        size_t sizeOfrequest = sizeOfStatus200 + sizeOfType + sizeOfLength + sizeOfConnection + sizeOfLineBreak + sizeOfEmptyTask;
        char* buildedResponse = (char*)malloc(sizeOfrequest * sizeof(char) + 1);

        strcat(buildedResponse, status);
        strcat(buildedResponse, type);
        strcat(buildedResponse, length);
        strcat(buildedResponse, connection);
        strcat(buildedResponse, lineBreak);
        strcat(buildedResponse, bodyEmptyTask);

        printf("%s \n", buildedResponse); //show full response
        free(charSizeEmptyTask);
        return buildedResponse;
    }
    if(id == 1)
    {
        size_t sizeOfrequest = sizeOfStatus200 + sizeOfConnection + sizeOfLineBreak;
        char* buildedResponse = (char*)malloc(sizeOfrequest * sizeof(char));

        strcat(buildedResponse, status);
        strcat(buildedResponse, connection);
        strcat(buildedResponse, lineBreak);

        printf("FULL REPONSE\n%s \n", buildedResponse); //show full response
        return buildedResponse;
    }
    return NULL;
}

void SendResponse(int clientSocket, char* buildedResponse)
{   
    size_t sizeOfBuildedResponse = strlen(buildedResponse);
    char response[sizeOfBuildedResponse];
    unsigned i = 0;
    while(i != sizeOfBuildedResponse)
    {
        response[i] = buildedResponse[i];
        i++;
    }
    printf("REPONSE IN SENDRESPONSE\n%s\n", response);

    send(clientSocket, response, sizeof(response), 0);
    free(buildedResponse);
}