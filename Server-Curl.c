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
void ParseName(parse_t* parseRequest);
int MethodID(parse_t* parseRequest);
task_t* Post (int socketClient, parse_t* parseRequest, task_t* node);
task_t* MethodInterractive(int socketClient, task_t* head, parse_t* parseRequest);
void Get(int socketClient, task_t* head, parse_t* ParseRequest);
char* BuildNames(char* buildNames, task_t* pointer);
char* BuildResponse(int id, char* names);
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

    task_t* head = NULL;
    while(true)
    {
        int clientSocket = accept(serverSocket, NULL, NULL);

        parse_t* parseRequest = (parse_t *)malloc(sizeof(parse_t));

        ParseName(ParseMethod(Recieve(clientSocket, parseRequest)));

        head = MethodInterractive(clientSocket, head, parseRequest);

        free(parseRequest->request);
        free(parseRequest->name);
        free(parseRequest);
    }

}
parse_t* Recieve(int clientSocket, parse_t* parseRequest)
{
    char buffer[1000] = "\0";
    recv(clientSocket, buffer, sizeof(buffer), 0);
    size_t sizeOfBuffer = strlen(buffer);

    parseRequest->request = malloc(sizeOfBuffer);
    strcpy(parseRequest->request, buffer);

    return parseRequest;
    //printf("Request:\n %s\n ", parseRequest->request);  //show full request and size
}

parse_t* ParseMethod(parse_t* parseRequest)
{
    char* request = parseRequest->request;
    char buffer[100] = "\0";

    int i = 0;

    while(request[i] != ' ')
    {
        buffer[i] = request[i];
        i++;
    }
    size_t sizeOfBuffer = strlen(buffer);
    parseRequest->method = malloc(sizeOfBuffer);
    strcpy(parseRequest->method, buffer);

    return parseRequest;
    //printf("Method \n%s\n", parseRequest->method);   //show method
}

char* IsolateBody(parse_t* parseRequest)
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

void ParseName(parse_t* parseRequest)
{
    if(strcmp(parseRequest->method, "POST") == 0)
    {
        char* body = IsolateBody(parseRequest);
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

        parseRequest->name = malloc(sizeOfBuffer);
        strcpy(parseRequest->name, buffer);

        free(body);
    }
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

        head->name = malloc(strlen(parseRequest->name));
        strcpy(head->name, parseRequest->name);

        head->nextTask = NULL;
        
        SendResponse(socketClient, BuildResponse(1, NULL));

        return head;
    }

    static int i = 1;

    task_t* pointer = head;
    while(pointer->nextTask != NULL)
    {
        pointer = pointer->nextTask;
    }

    pointer->nextTask = (task_t*)malloc(sizeof(task_t));
    pointer->nextTask->id = i++;

    pointer->nextTask->name = malloc(strlen(parseRequest->name));
    strcpy(pointer->nextTask->name, parseRequest->name);
    pointer->nextTask->nextTask = NULL;

    return head;
}

void Get(int socketClient, task_t* head, parse_t* parseRequest)
{
    if(head == NULL)
    {   
        SendResponse(socketClient, BuildResponse(0, NULL));
        return;
    }
    task_t* pointer = head;

    char* buildedNames = NULL;
    while(pointer != NULL)
    {
        buildedNames = BuildNames(buildedNames, pointer);
        pointer = pointer->nextTask;
    }
    SendResponse(socketClient, BuildResponse(2, buildedNames));

    printf("\n\nbuildedNames\n %s\n\n", buildedNames);
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

char* BuildResponse(int id, char* names)
{
    char bodyEmptyTask[] = "The task is empty";
    size_t sizeOfEmptyTask = strlen(bodyEmptyTask);
    
    char status[] = "HTTP/1.1 200 OK \n";
    size_t sizeOfStatus200 = strlen(status);

    char type[] = "Content-Type: application/json \n";
    size_t sizeOfType = strlen(type);

    char length[] = "Content-Length: ";
    size_t sizeOfLength = strlen(length);

    char connection[] = "Connection: keep-alive \n";
    size_t sizeOfConnection = strlen(connection);

    char lineBreak[] = "\n";
    size_t sizeOfLineBreak = strlen(lineBreak);


    if(id == 0)
    {
        char* charSizeEmptyTask = malloc(sizeOfEmptyTask + 1);
        sprintf(charSizeEmptyTask, "%ld", sizeOfEmptyTask);

        char* ContentLength = malloc(sizeOfLength + sizeOfEmptyTask + sizeOfLineBreak + 1); 
        strcat(ContentLength, length);
        strcat(ContentLength, charSizeEmptyTask);
        strcat(ContentLength, lineBreak);
        
        size_t sizeOfrequest = sizeOfStatus200 + sizeOfType + sizeOfLength + sizeOfConnection + sizeOfLineBreak + sizeOfEmptyTask;
        char* buildedResponse = malloc(sizeOfrequest + 1);

        strcat(buildedResponse, status);
        strcat(buildedResponse, type);
        strcat(buildedResponse, ContentLength);
        strcat(buildedResponse, connection);
        strcat(buildedResponse, lineBreak);
        strcat(buildedResponse, bodyEmptyTask);

        //printf("%s \n", buildedResponse); //show full response
        free(charSizeEmptyTask);
        return buildedResponse;
    }
    if(id == 1)
    {
        size_t sizeOfrequest = sizeOfStatus200 + sizeOfConnection + sizeOfLineBreak;
        char* buildedResponse = (char*)malloc(sizeOfrequest + 1);

        strcat(buildedResponse, status);
        strcat(buildedResponse, connection);
        strcat(buildedResponse, lineBreak);

        //printf("\nFull response\n%s \n", buildedResponse); //show full response
        return buildedResponse;
    }
    if(id == 2)
    {
        size_t sizeOfNames = strlen(names);
        size_t sizeOfResponse = sizeOfStatus200 + sizeOfType + sizeOfLength + sizeOfConnection + sizeOfLineBreak + sizeOfNames;

        char* charSizeOfNames = malloc(sizeOfNames);
        sprintf(charSizeOfNames, "%ld", sizeOfNames);

        strcat(length, charSizeOfNames);
        strcat(length, lineBreak);

        char* buildedResponse = malloc(sizeOfResponse + 1);
       
        strcat(buildedResponse, status);
        strcat(buildedResponse, type);
        strcat(buildedResponse, length);
        strcat(buildedResponse, connection);
        strcat(buildedResponse, lineBreak);
        strcat(buildedResponse, names);

        return buildedResponse;
    }
    return NULL;
}

void SendResponse(int clientSocket, char* buildedResponse)
{   
    char buffer[1024] = "\0";
    strcpy(buffer, buildedResponse);
    size_t sizeOfBuildedResponse = strlen(buffer);

    char response[sizeOfBuildedResponse];
    unsigned i = 0;
    while(i != sizeOfBuildedResponse)
    {
        response[i] = buildedResponse[i];
        i++;
    }
    printf("\nResponse in SendResponse\n%s\n", response);

    send(clientSocket, response, sizeof(response), 0);
    free(buildedResponse);
}

char* BuildNames(char* buildedNames, task_t* pointer)
{
    int id = pointer->id;

    char* name = pointer->name;
    size_t sizeOfName = strlen(name);

    char responseId[] = "Id: ";
    size_t sizeOfRersosnseId = strlen(responseId);

    char responseName[] = "name: ";
    size_t sizeOfRersosnseName = strlen(responseName);

    char breakLine[] = "\n";
    size_t sizeOfBreakLine = strlen(breakLine);

    char space[] = " ";
    size_t sizeOfSpace = strlen(space);

    char* charId = calloc(id + 1, sizeof(char));
    sprintf(charId, "%d", id);
    size_t sizeOfCharId = strlen(charId);

    char* fullResponseId = calloc(sizeOfRersosnseId + sizeOfCharId + 1, sizeof(char));
    strcat(fullResponseId, responseId);
    strcat(fullResponseId, charId);
    size_t sizeOfFullResponseId = strlen(fullResponseId);

    char* fullResponseName = calloc(sizeOfRersosnseName + sizeOfName + 1, sizeof(char));

    strcat(fullResponseName, responseName);
    strcat(fullResponseName, name);
    size_t sizeOfFullResponseName = strlen(fullResponseName);

    if(buildedNames == NULL)
    {
        buildedNames = calloc(sizeOfFullResponseId + sizeOfSpace + sizeOfFullResponseName + sizeOfBreakLine + 1, sizeof(char));
    }
    else
    {
        size_t sizeOfBuildedNames = strlen(buildedNames);
        char* bufferBuildedNames = calloc(sizeOfBuildedNames + sizeOfFullResponseId + sizeOfSpace + sizeOfFullResponseName + sizeOfBreakLine + 1, sizeof(char));
        strcat(bufferBuildedNames, buildedNames);
        free(buildedNames);
        buildedNames = bufferBuildedNames;
    }
    
    strcat(buildedNames, fullResponseId);
    strcat(buildedNames, space);
    strcat(buildedNames, fullResponseName);
    strcat(buildedNames, breakLine);
    
    free(charId);
    free(fullResponseId);
    free(fullResponseName);
    
    return buildedNames;
}