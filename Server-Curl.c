#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>

typedef struct Parse
{
    char* request;
}parse_t;

typedef struct Task
{
    int id;
    char* name;
    struct Task * nextTask;
}task_t;

void SetSocket();
parse_t* Recieve(int clientSocket, parse_t* parseRequest);
char* ParseMethod(parse_t* parseRequest);
char* IsolateBody(parse_t* parseRequest);
char* ParseName(parse_t* parseRequest);
int ParseId(parse_t* parseRequest);
int MethodID(parse_t* parseRequest);
task_t* Create (int socketClient, parse_t* parseRequest, task_t* node);
task_t* MethodInterractive(int socketClient, task_t* head, parse_t* parseRequest);
void Read(int socketClient, task_t* head, parse_t* ParseRequest);
char* BuildNames(char* buildNames, task_t* pointer);
char* BuildResponse(int id, char* names);
void SendResponse(int clientSocket, char* request);
void Update(int clientSocket, task_t* head, parse_t* ParseRequest);
task_t* Delete(int socketClient, task_t* head, parse_t* parseRequest);

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
    parse_t* parseRequest = (parse_t *)malloc(sizeof(parse_t));
    while(true)
    {
        int clientSocket = accept(serverSocket, NULL, NULL);

        Recieve(clientSocket, parseRequest);

        head = MethodInterractive(clientSocket, head, parseRequest);

        free(parseRequest->request);
    }
}
parse_t* Recieve(int clientSocket, parse_t* parseRequest)
{
    char buffer[1000] = "\0";
    recv(clientSocket, buffer, sizeof(buffer), 0);
    size_t sizeOfBuffer = strlen(buffer);

    parseRequest->request = calloc(sizeOfBuffer + 1, sizeof(char));
    strcpy(parseRequest->request, buffer);

    //printf("\nRequest:\n %s\n ", parseRequest->request);  //show full request
    return parseRequest;
}

char* ParseMethod(parse_t* parseRequest)
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
    char* method = calloc(sizeOfBuffer + 1, sizeof(char));
    strcpy(method, buffer);
    //printf("Method \n%s\n", parseRequest->method);   //show method
    return method;
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
    char* body = calloc(sizeOfBuffer + 1, sizeof(char));
    strcpy(body, buffer);

    return body;
}

char* ParseName(parse_t* parseRequest)
{
    char* body = IsolateBody(parseRequest);
    size_t sizeOfBody = strlen(body);

    int i = 0;

    if(strcmp(ParseMethod(parseRequest), "DELETE") == 0 || strcmp(ParseMethod(parseRequest), "PUT") == 0)
    {
        while(i != sizeOfBody)
        {
            if(body[i] == ',')
            {
                break;
            }
            i++;
        }
    }
    
    while(i != sizeOfBody)
    {
        if(body[i] == ':')
        {
            break;
        }
        i++;
    }

    while(i != sizeOfBody)
    {
        if(body[i] == '\"')
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

    char* name = calloc(sizeOfBuffer + 1, sizeof(char));
    strcpy(name, buffer);

    //printf("\nname %s\n", name);
    free(body);
    return name;
}

int ParseId(parse_t* parseRequest)
{
    char* body = IsolateBody(parseRequest);
    size_t sizeOfBody = strlen(body);
    
    int i = 0;
    while(i < sizeOfBody)
    {
        if(body[i] == ':')
        {
            break;
        }
        i++;
    }

    while(i < sizeOfBody)
    {
        if(body[i] == '\"')
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
        if(body[i] == '\"')
        {
            break;
        }
        buffer[j] = body[i];
        i++;
        j++;
    }

    size_t sizeOfBuffer = strlen(buffer);
    char* charId = calloc(sizeOfBuffer + 1, sizeof(char));
    strcat(charId, buffer);
    int id = atoi(charId);

    //printf("\n\nId %d\n\n", id);
    return id;
}

int MethodID(parse_t* ParseRequest)
{
    if(strcmp(ParseMethod(ParseRequest), "GET") == 0)
    {
        return 0;
    }
    if(strcmp(ParseMethod(ParseRequest), "POST") == 0)
    {
        return 1;
    }
    if(strcmp(ParseMethod(ParseRequest), "PUT") == 0)
    {
        return 2;
    }
    if(strcmp(ParseMethod(ParseRequest), "DELETE") == 0)
    {
        return 3;
    }

    return -1;
}



task_t* MethodInterractive(int socketClient, task_t* head, parse_t* parseRequest)
{
    switch (MethodID(parseRequest))
    {
        case 0:
            Read(socketClient, head, parseRequest);
            break;
        case 1:
            return Create(socketClient, parseRequest, head);
        case 2:
            Update(socketClient, head, parseRequest);
            break;
        case 3:
            return Delete(socketClient, head, parseRequest);
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
        char* charSizeEmptyTask = calloc(sizeOfEmptyTask + 1, sizeof(char));
        sprintf(charSizeEmptyTask, "%ld", sizeOfEmptyTask);

        char* ContentLength = calloc(sizeOfLength + sizeOfEmptyTask + sizeOfLineBreak + 1, sizeof(char)); 
        strcat(ContentLength, length);
        strcat(ContentLength, charSizeEmptyTask);
        strcat(ContentLength, lineBreak);
        
        size_t sizeOfrequest = sizeOfStatus200 + sizeOfType + sizeOfLength + sizeOfConnection + sizeOfLineBreak + sizeOfEmptyTask;
        char* buildedResponse = calloc(sizeOfrequest + 1, sizeof(char));

        strcat(buildedResponse, status);
        strcat(buildedResponse, type);
        strcat(buildedResponse, ContentLength);
        strcat(buildedResponse, connection);
        strcat(buildedResponse, lineBreak);
        strcat(buildedResponse, bodyEmptyTask);

        //printf("%s \n", buildedResponse); //show full response
        free(charSizeEmptyTask);
        free(ContentLength);

        return buildedResponse;
    }
    if(id == 1)
    {
        size_t sizeOfrequest = sizeOfStatus200 + sizeOfConnection + sizeOfLineBreak + 1;
        char* buildedResponse = calloc(sizeOfrequest + 1, sizeof(char));

        strcat(buildedResponse, status);
        strcat(buildedResponse, connection);
        strcat(buildedResponse, lineBreak);

        //printf("\nFull response\n%s \n", buildedResponse); //show full response
        return buildedResponse;
    }
    if(id == 2)
    {
        size_t sizeOfNames = strlen(names);

        char* charSizeOfNames = calloc(sizeOfNames + 1, sizeof(char));
        sprintf(charSizeOfNames, "%ld", sizeOfNames);

        char* ContentLength = calloc(sizeOfLength + sizeOfNames + sizeOfLineBreak + 1, sizeof(char));
        strcat(ContentLength, length);
        strcat(ContentLength, charSizeOfNames);
        strcat(ContentLength, lineBreak);
        size_t sizeOfContentLength = strlen(ContentLength);

        size_t sizeOfResponse = sizeOfStatus200 + sizeOfType + sizeOfContentLength + sizeOfConnection + sizeOfLineBreak + sizeOfNames + 1;
        char* buildedResponse = calloc(sizeOfResponse + 1, sizeof(char));
       
        strcat(buildedResponse, status);
        strcat(buildedResponse, type);
        strcat(buildedResponse, ContentLength);
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
    //printf("\nResponse in SendResponse\n%s\n", response);

    send(clientSocket, response, sizeof(response), 0);
    free(buildedResponse);
}

char* BuildNames(char* buildedNames, task_t* pointer)
{
    int id = pointer->id;

    char* name = pointer->name;
    size_t sizeOfName = strlen(name);

    char responseId[] = "Id";
    size_t sizeOfRersosnseId = strlen(responseId);

    char responseName[] = "name";
    size_t sizeOfRersosnseName = strlen(responseName);

    char breakLine[] = "\n";
    size_t sizeOfBreakLine = strlen(breakLine);

    char space[] = " ";
    size_t sizeOfSpace = strlen(space);

    char* charId = calloc(id + 1, sizeof(char));
    sprintf(charId, "%d", id);
    size_t sizeOfCharId = strlen(charId);

    char openedBracket[] = "{";
    size_t sizeOfOpenedBracket = strlen(openedBracket);

    char closedBracket[] = "}";
    size_t sizeOfClosedBracket = strlen(closedBracket);

    char quotes[] = "\"";
    size_t sizeOfQuotes = strlen(quotes);

    char colon[] = ":";
    size_t sizeOfColon = strlen(colon);

    char comma[] = ",";
    size_t sizeOfComma = strlen(comma);
    

    char* fullResponseId = calloc(sizeOfQuotes + sizeOfRersosnseId + sizeOfQuotes + sizeOfColon + sizeOfSpace + sizeOfQuotes + sizeOfCharId + sizeOfQuotes + 1, sizeof(char));

    strcat(fullResponseId, quotes);
    strcat(fullResponseId, responseId);
    strcat(fullResponseId, quotes);
    strcat(fullResponseId, colon);
    strcat(fullResponseId, space);
    strcat(fullResponseId, quotes);
    strcat(fullResponseId, charId);
    strcat(fullResponseId, quotes);
    
    size_t sizeOfFullResponseId = strlen(fullResponseId);

    char* fullResponseName = calloc(sizeOfQuotes + sizeOfRersosnseName + sizeOfQuotes + sizeOfColon + sizeOfSpace + sizeOfComma + sizeOfName + sizeOfComma + 1, sizeof(char));

    strcat(fullResponseName, quotes);
    strcat(fullResponseName, responseName);
    strcat(fullResponseName, quotes);
    strcat(fullResponseName, colon);
    strcat(fullResponseName, space);
    strcat(fullResponseName, quotes);
    strcat(fullResponseName, name);
    strcat(fullResponseName, quotes);
    
    size_t sizeOfFullResponseName = strlen(fullResponseName);

    if(buildedNames == NULL)
    {
        size_t sizeOfBuiledNames = sizeOfOpenedBracket + sizeOfFullResponseId + sizeOfFullResponseName + sizeOfClosedBracket + sizeOfBreakLine + 1;
        buildedNames = calloc(sizeOfBuiledNames, sizeof(char));
    }
    else
    {
        size_t sizeOfBuildedNames = strlen(buildedNames);

        size_t sizeOfBufferBuildedNames = sizeOfBuildedNames + sizeOfOpenedBracket + sizeOfFullResponseId + sizeOfComma  + sizeOfSpace + sizeOfFullResponseName + sizeOfClosedBracket + sizeOfBreakLine + 1;
        char* bufferBuildedNames = calloc(sizeOfBufferBuildedNames + 1, sizeof(char));
        strcat(bufferBuildedNames, buildedNames);

        free(buildedNames);

        buildedNames = bufferBuildedNames;
    }
    strcat(buildedNames, openedBracket);
    strcat(buildedNames, fullResponseId);
    strcat(buildedNames, comma);
    strcat(buildedNames, space);
    strcat(buildedNames, fullResponseName);
    strcat(buildedNames, closedBracket);
    strcat(buildedNames, breakLine);
    
    free(charId);
    free(fullResponseId);
    free(fullResponseName);
    
    return buildedNames;
}

task_t* Create (int socketClient, parse_t* parseRequest, task_t* head)
{
    char* name = ParseName(parseRequest);
    if(head == NULL)
    {
        head = (task_t*)malloc(sizeof(task_t));
        head->id = 0;

        head->name = calloc(strlen(name) + 1, sizeof(char));
        strcpy(head->name, name);

        head->nextTask = NULL;
        
        SendResponse(socketClient, BuildResponse(1, NULL));

        return head;
    }

    int i = 1;

    task_t* pointer = head;
    while(pointer->nextTask != NULL)
    {
        pointer = pointer->nextTask;
        i++;
    }

    pointer->nextTask = malloc(sizeof(task_t));
    pointer->nextTask->id = i++;

    pointer->nextTask->name = calloc(strlen(name) + 1, sizeof(char));
    strcpy(pointer->nextTask->name, name);
    pointer->nextTask->nextTask = NULL;

    return head;
}

void Read(int socketClient, task_t* head, parse_t* parseRequest)
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

    //printf("\n\nbuildedNames\n %s\n\n", buildedNames);
}

void Update(int socketClient, task_t* head, parse_t* parseRequest)
{
    if(head == NULL)
    {
        SendResponse(socketClient, BuildResponse(0, NULL));
        return;
    }
    task_t* pointer = head;

    int id = ParseId(parseRequest);
    char* newName = ParseName(parseRequest);

    while(pointer->id != id)
    {
        pointer = pointer->nextTask;
    }
    free(pointer->name);
    pointer->name = calloc(strlen(newName) + 1, sizeof(char));
    strcat(pointer->name, newName);

    SendResponse(socketClient, BuildResponse(1, NULL));
}
task_t* Delete(int socketClient, task_t* head, parse_t* parseRequest)
{
    int id = ParseId(parseRequest);

    task_t* lastTask = head;
    while(lastTask->nextTask != NULL)
    {
        lastTask = lastTask->nextTask;
    }
    if(id < head->id || id > lastTask->id)
    {
        return head; // must be warning for client
    }

    task_t* pointer = head;
    if(id == head->id)
    {
        pointer->nextTask->id = head->id;
        head = pointer->nextTask;
        free(pointer);
        pointer = head;
    }

    if(id > head->id && id < lastTask->id)
    {
        while(pointer->nextTask->id != id)
        {
            pointer = pointer->nextTask;
        }
        task_t* deleteTask = pointer->nextTask;
        pointer->nextTask = deleteTask->nextTask;
        free(deleteTask);
        pointer = pointer->nextTask;
    }

    if(id == lastTask->id)
    {
        while(pointer->nextTask != lastTask)
        {
            pointer = pointer->nextTask;
        }
        free(pointer->nextTask);
        pointer->nextTask = NULL;
    }

    while(pointer->nextTask != NULL)
    {
        pointer->nextTask->id = pointer->nextTask->id - 1;
        pointer = pointer->nextTask;
    }

    return head;
}