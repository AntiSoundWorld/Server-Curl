#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>

typedef struct List
{
    char* bodyEmptyTask;
    char* status;
    char* type;
    char* length;
    char* connection;
    char* lineBreak;
    char* responseId;
    char* responseName;
    char* space;
    char* openedBracket;
    char* closedBracket;
    char* quotes;
    char* colon;
    char* comma;
}list_t;
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
int ParseId(int i, parse_t* parseRequest);
char* ParseBody(int i, parse_t* parseRequest);
task_t* Create (int socketClient, parse_t* parseRequest, task_t* node);
task_t* MethodInterractive(int socketClient, task_t* head, parse_t* parseRequest);
void Read(int socketClient, task_t* head, parse_t* ParseRequest);
char* BuildNames(char* buildNames, task_t* pointer);
char* BuildResponse(int id, char* names);
void SendResponse(int clientSocket, char* request);
void Update(int clientSocket, task_t* head, parse_t* ParseRequest);
task_t* Delete(int socketClient, task_t* head, parse_t* parseRequest);
list_t* List();
void FreeList(list_t* list);
void ReassigneId(task_t* head);
char* CheckNameExistance(parse_t* parseRequest);
int CheckIdExistance(parse_t* parseRequest);
char* BuildResponseRead(list_t* list, char* names);
char* BuildConfirmationRequest(list_t* list);
char* BuildResponseEmptyTask(list_t* list);

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

char* ParseBody(int i, parse_t* parseRequest)
{
    char* body = IsolateBody(parseRequest);
    size_t sizeOfBody = strlen(body);

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


task_t* MethodInterractive(int socketClient, task_t* head, parse_t* parseRequest)
{
    char* method = ParseMethod(parseRequest);
    
    if(strcmp(method, "GET") == 0)
    {
        Read(socketClient, head, parseRequest);
    }
    if(strcmp(method, "POST") == 0)
    {
        return Create(socketClient, parseRequest, head);
    }
    if(strcmp(method, "PUT") == 0)
    {
        Update(socketClient, head, parseRequest);
    }
    if(strcmp(method, "DELETE") == 0)
    {
        return Delete(socketClient, head, parseRequest);
    }
    free(method);

    return head;
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
    list_t* list = List();

    int id = pointer->id;
    char* charId = calloc(id + 1, sizeof(char));
    sprintf(charId, "%d", id);

    char* name = pointer->name;
    size_t sizeOfName = strlen(name);

    char* fullResponseId = calloc(strlen(list->quotes) + strlen(list->responseId) + strlen(list->quotes) + strlen(list->colon) 
    + strlen(list->space) + strlen(list->quotes) + strlen(charId) + strlen(list->quotes) + 1, sizeof(char));

    strcat(fullResponseId, list->quotes);
    strcat(fullResponseId, list->responseId);
    strcat(fullResponseId, list->quotes);
    strcat(fullResponseId, list->colon);
    strcat(fullResponseId, list->space);
    strcat(fullResponseId, list->quotes);
    strcat(fullResponseId, charId);
    strcat(fullResponseId, list->quotes);
    

    char* fullResponseName = calloc(strlen(list->quotes) + strlen(list->responseName) + strlen(list->quotes) + strlen(list->colon) 
    + strlen(list->space) + strlen(list->comma) + sizeOfName + strlen(list->comma) + 1, sizeof(char));

    strcat(fullResponseName, list->quotes);
    strcat(fullResponseName, list->responseName);
    strcat(fullResponseName, list->quotes);
    strcat(fullResponseName, list->colon);
    strcat(fullResponseName, list->space);
    strcat(fullResponseName, list->quotes);
    strcat(fullResponseName, name);
    strcat(fullResponseName, list->quotes);
    
    if(buildedNames == NULL)
    {
        size_t sizeOfBuiledNames = strlen(list->openedBracket) + strlen(fullResponseId) + strlen(fullResponseName) 
        + strlen(list->closedBracket) + strlen(list->lineBreak) + 1;

        buildedNames = calloc(sizeOfBuiledNames, sizeof(char));
    }
    else
    {
        size_t sizeOfBufferBuildedNames = strlen(buildedNames) + strlen(list->openedBracket) + strlen(fullResponseId) + strlen(list->comma) 
        + strlen(list->space) + strlen(fullResponseName) + strlen(list->closedBracket) + strlen(list->lineBreak) + 1;

        char* bufferBuildedNames = calloc(sizeOfBufferBuildedNames + 1, sizeof(char));
        strcat(bufferBuildedNames, buildedNames);

        free(buildedNames);

        buildedNames = bufferBuildedNames;
    }
    strcat(buildedNames, list->openedBracket);
    strcat(buildedNames, fullResponseId);
    strcat(buildedNames, list->comma);
    strcat(buildedNames, list->space);
    strcat(buildedNames, fullResponseName);
    strcat(buildedNames, list->closedBracket);
    strcat(buildedNames, list->lineBreak);
    
    free(charId);
    free(fullResponseId);
    free(fullResponseName);
    FreeList(list);

    return buildedNames;
}

task_t* Create (int socketClient, parse_t* parseRequest, task_t* head)
{
    char* name = CheckNameExistance(parseRequest);
    if(name == NULL)
    {
        //place for warning
        return head;
    }
    if(head == NULL)
    {
        head = (task_t*)malloc(sizeof(task_t));
        head->id = 0;

        head->name = calloc(strlen(name) + 1, sizeof(char));
        strcpy(head->name, name);
        free(name);

        head->nextTask = NULL;
        
        SendResponse(socketClient, BuildConfirmationRequest(List()));

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
        SendResponse(socketClient, BuildResponseEmptyTask(List()));
        return;
    }
    task_t* pointer = head;

    char* buildedNames = NULL;
    while(pointer != NULL)
    {
        buildedNames = BuildNames(buildedNames, pointer);
        pointer = pointer->nextTask;
    }
    SendResponse(socketClient, BuildResponseRead(List(), buildedNames));

    //printf("\n\nbuildedNames\n %s\n\n", buildedNames);
}

void Update(int socketClient, task_t* head, parse_t* parseRequest)
{
    if(head == NULL)
    {
        SendResponse(socketClient, BuildResponseEmptyTask(List()));
        return;
    }
    task_t* pointer = head;

    int id = CheckIdExistance(parseRequest);
    char* newName = CheckNameExistance(parseRequest);
    if(id == -1 || newName == NULL)
    {
        //place for warning
        return;
    }

    while(pointer->id != id)
    {
        pointer = pointer->nextTask;
    }
    free(pointer->name);
    
    pointer->name = calloc(strlen(newName) + 1, sizeof(char));
    strcat(pointer->name, newName);
    free(newName);

    SendResponse(socketClient, BuildConfirmationRequest(List()));
}

task_t* Delete(int socketClient, task_t* head, parse_t* parseRequest)
{
    int id = CheckIdExistance(parseRequest);
    if(id == -1)
    {
        //place for warning
        return head;
    }
    
    task_t* lastTask = head;
    while(lastTask->nextTask != NULL)
    {
        lastTask = lastTask->nextTask;
    }
    

    if(id < head->id || id > lastTask->id)
    {
        return head; // place for warning
    }

    printf("%d", id);

    task_t* pointer = head;
    if(id == 0)
    {
        head = pointer->nextTask;
        free(pointer);
        pointer = head;
        ReassigneId(head);

        SendResponse(socketClient, BuildConfirmationRequest(List()));
        return head;
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

    ReassigneId(head);

    SendResponse(socketClient, BuildConfirmationRequest(List()));
    return head;
}

list_t* List()
{
    list_t* list = malloc(sizeof(list_t));

    char bodyEmptyTask[] = "The task is empty";
    list->bodyEmptyTask = calloc(strlen(bodyEmptyTask) + 1, sizeof(char));
    strcpy(list->bodyEmptyTask, bodyEmptyTask);

    char status[] = "HTTP/1.1 200 OK \n";
    list->status = calloc(strlen(status) + 1, sizeof(char));
    strcpy(list->status, status);

    char type[] = "Content-Type: application/json \n";
    list->type = calloc(strlen(type) + 1, sizeof(char));
    strcpy(list->type, type);

    char length[] = "Content-Length: ";
    list->length = calloc(strlen(length) + 1, sizeof(char));
    strcpy(list->length, length);

    char connection[] = "Connection: keep-alive \n";
    list->connection = calloc(strlen(connection) + 1, sizeof(char));
    strcpy(list->connection, connection);

    char lineBreak[] = "\n";
    list->lineBreak = calloc(strlen(lineBreak) + 1, sizeof(char));
    strcpy(list->lineBreak, lineBreak);

    char responseId[] = "Id";
    list->responseId = calloc(strlen(responseId) + 1, sizeof(char));
    strcpy(list->responseId, responseId);

    char responseName[] = "name";
    list->responseName = calloc(strlen(responseName) + 1, sizeof(char));;
    strcpy(list->responseName, responseName);

    char space[] = " ";
    list->space = calloc(strlen(space) + 1, sizeof(char));;
    strcpy(list->space, space);

    char openedBracket[] = "{";
    list->openedBracket = calloc(strlen(openedBracket) + 1, sizeof(char));;
    strcpy(list->openedBracket, openedBracket);

    char closedBracket[] = "}";
    list->closedBracket = calloc(strlen(closedBracket) + 1, sizeof(char));;;
    strcpy(list->closedBracket, closedBracket);

    char quotes[] = "\"";
    list->quotes = calloc(strlen(quotes) + 1, sizeof(char));;
    strcpy(list->quotes, quotes);

    char colon[] = ":";
    list->colon = calloc(strlen(colon) + 1, sizeof(char));;
    strcpy(list->colon, colon);

    char comma[] = ",";
    list->comma = calloc(strlen(comma) + 1, sizeof(char));;
    strcpy(list->comma, comma);

    return list;
}

void FreeList(list_t* list)
{
    free(list->bodyEmptyTask);
    free(list->status);
    free(list->type);
    free(list->length);
    free(list->connection);
    free(list->lineBreak);
    free(list->responseId);
    free(list->responseName);
    free(list->space);
    free(list->openedBracket);
    free(list->closedBracket);
    free(list->quotes);
    free(list->colon);
    free(list->comma);
    free(list);
}

void ReassigneId(task_t* head)
{
    task_t* pointer = head;
    int i = 0;
    while(pointer != NULL)
    {
        pointer->id = i;
        pointer = pointer->nextTask;
        i++;
    }
}

int CheckIdExistance(parse_t* parseRequest)
{
    char* body = IsolateBody(parseRequest);
    size_t sizeOfBody = strlen(body);

    int i = 0;
    while(i != sizeOfBody - 2)
    {
        if(body[i] == 'i' && body[i + 1] == 'd')
        {
            return atoi(ParseBody(i, parseRequest));
        }
        i++;
    }
    return -1;
}

char* CheckNameExistance(parse_t* parseRequest)
{
    char* body = IsolateBody(parseRequest);
    size_t sizeOfBody = strlen(body);

    int i = 0;
    while(i != sizeOfBody - 3)
    {
        if(body[i] == 'n' && body[i + 1] == 'a' && body[i + 2] == 'm' && body[i + 3] == 'e')
        {
           return ParseBody(i, parseRequest);
        }
        i++;
    }
    return NULL;
}

char* BuildResponseEmptyTask(list_t* list)
{
    char* charSizeEmptyTask = calloc(strlen(list->bodyEmptyTask) + 1, sizeof(char));
    sprintf(charSizeEmptyTask, "%ld", strlen(list->bodyEmptyTask));

    char* contentLength = calloc(strlen(list->length) + strlen(list->bodyEmptyTask) + strlen(list->lineBreak) + 1, sizeof(char)); 
    strcat(contentLength, list->length);
    strcat(contentLength, charSizeEmptyTask);
    strcat(contentLength, list->lineBreak);
    
    size_t sizeOfrequest = strlen(list->status) + strlen(list->type) + strlen(contentLength) + strlen(list->connection) 
    + strlen(list->lineBreak) + strlen(list->bodyEmptyTask);

    char* buildedResponse = calloc(sizeOfrequest + 1, sizeof(char));

    strcat(buildedResponse, list->status);
    strcat(buildedResponse, list->type);
    strcat(buildedResponse, contentLength);
    strcat(buildedResponse, list->connection);
    strcat(buildedResponse, list->lineBreak);
    strcat(buildedResponse, list->bodyEmptyTask);

    //printf("%s \n", buildedResponse); //show full response
    free(charSizeEmptyTask);
    free(contentLength);
    FreeList(list);
    return buildedResponse;
}

char* BuildConfirmationRequest(list_t* list)
{
    size_t sizeOfrequest = strlen(list->status) + strlen(list->connection) + strlen(list->lineBreak) + 1;
    char* buildedResponse = calloc(sizeOfrequest + 1, sizeof(char));

    strcat(buildedResponse, list->status);
    strcat(buildedResponse, list->connection);
    strcat(buildedResponse, list->lineBreak);

    FreeList(list);
    //printf("\nFull response\n%s \n", buildedResponse); //show full response
    return buildedResponse;
}
char* BuildResponseRead(list_t* list, char* names)
{
    size_t sizeOfNames = strlen(names);

    char* charSizeOfNames = calloc(sizeOfNames + 1, sizeof(char));
    sprintf(charSizeOfNames, "%ld", sizeOfNames);

    char* ContentLength = calloc(strlen(list->length) + sizeOfNames + strlen(list->lineBreak) + 1, sizeof(char));
    strcat(ContentLength, list->length);
    strcat(ContentLength, charSizeOfNames);
    strcat(ContentLength, list->lineBreak);
    size_t sizeOfContentLength = strlen(ContentLength);

    size_t sizeOfResponse = strlen(list->status) + strlen(list->type) + sizeOfContentLength + strlen(list->connection) 
    + strlen(list->lineBreak) + sizeOfNames + 1;

    char* buildedResponse = calloc(sizeOfResponse + 1, sizeof(char));

    strcat(buildedResponse, list->status);
    strcat(buildedResponse, list->type);
    strcat(buildedResponse, ContentLength);
    strcat(buildedResponse, list->connection);
    strcat(buildedResponse, list->lineBreak);
    strcat(buildedResponse, names);

    FreeList(list);

    return buildedResponse;
}
