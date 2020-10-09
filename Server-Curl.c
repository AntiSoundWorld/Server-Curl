#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>

typedef struct List
{
    char* unknownFieldId;
    char* bodyEmptyTask;
    char* status200;
    char* status201;
    char* status400;
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
    char* errorValue;
    char* error;
    char* errorId;
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
char* FindName(parse_t* parseRequest);
int FindId(parse_t* parseRequest);
char* BuildResponseRead(list_t* list, char* names);
char* BuildResponseConfirmationRequest(list_t* list, int id);
char* BuildResponseEmptyTask(list_t* list);
char* BuildResponseErrorValue(list_t* list);
char* BuildResponseErrorId(list_t* list);
bool CheckErrorId(int socketClient, task_t* head, parse_t* parseRequest);
bool CheckErrorName(int socketClient, parse_t* parseRequest);

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

    char* value = calloc(sizeOfBuffer + 1, sizeof(char));
    strcpy(value, buffer);

    //printf("\nParseBody value = %s\n", value);
    free(body);

    return value;
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
    char buffer[1024] = "\0";
    sprintf(buffer, "%d", id);

    char* charId = calloc(strlen(buffer) + 1, sizeof(char));
    strcat(charId, buffer);

    char* name = pointer->name;
    size_t sizeOfName = strlen(name);

    char* fullResponseId = calloc(strlen(list->quotes) + strlen(list->responseId) + strlen(list->quotes) + strlen(list->colon) 
    + strlen(list->quotes) + strlen(charId) + strlen(list->quotes) + 1, sizeof(char));

    strcat(fullResponseId, list->quotes);
    strcat(fullResponseId, list->responseId);
    strcat(fullResponseId, list->quotes);
    strcat(fullResponseId, list->colon);
    strcat(fullResponseId, list->quotes);
    strcat(fullResponseId, charId);
    strcat(fullResponseId, list->quotes);
    

    char* fullResponseName = calloc(strlen(list->quotes) + strlen(list->responseName) + strlen(list->quotes) + strlen(list->colon) 
    + strlen(list->quotes) + sizeOfName + strlen(list->quotes) + 1, sizeof(char));

    strcat(fullResponseName, list->quotes);
    strcat(fullResponseName, list->responseName);
    strcat(fullResponseName, list->quotes);
    strcat(fullResponseName, list->colon);
    strcat(fullResponseName, list->quotes);
    strcat(fullResponseName, name);
    strcat(fullResponseName, list->quotes);
    
    if(buildedNames == NULL)
    {
        size_t sizeOfBuiledNames = strlen(list->openedBracket) + strlen(fullResponseId) + strlen(list->comma) + strlen(fullResponseName)
        + strlen(list->closedBracket) + strlen(list->lineBreak);

        buildedNames = calloc(sizeOfBuiledNames + 1, sizeof(char));
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
    if(CheckErrorName(socketClient, parseRequest))
    {
        return head;
    }

    char* name = FindName(parseRequest);

    if(head == NULL)
    {
        head = (task_t*)malloc(sizeof(task_t));
        head->id = 0;

        head->name = calloc(strlen(name) + 1, sizeof(char));
        strcpy(head->name, name);
        free(name);

        head->nextTask = NULL;
        
        SendResponse(socketClient, BuildResponseConfirmationRequest(List(), head->id));

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

    SendResponse(socketClient, BuildResponseConfirmationRequest(List(), pointer->nextTask->id));

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
    if(CheckErrorId(socketClient, head, parseRequest) || CheckErrorName(socketClient, parseRequest))
    {
        return;
    }

    int id = FindId(parseRequest);
    
    char* newName = FindName(parseRequest);
    if(newName == NULL)
    {
        SendResponse(socketClient, BuildResponseErrorValue(List()));
        return;
    }

    task_t* pointer = head;

    while(pointer->id != id)
    {
        pointer = pointer->nextTask;
    }
    free(pointer->name);
    
    pointer->name = calloc(strlen(newName) + 1, sizeof(char));
    strcat(pointer->name, newName);
    free(newName);

    SendResponse(socketClient, BuildResponseConfirmationRequest(List(), -1));
}

task_t* Delete(int socketClient, task_t* head, parse_t* parseRequest)
{
    if(CheckErrorId(socketClient, head, parseRequest))
    {
        return head;
    }

    int id = FindId(parseRequest);

    task_t* pointer = head;
    if(id == 0)
    {
        head = pointer->nextTask;
        free(pointer);
        pointer = head;
        ReassigneId(head);

        SendResponse(socketClient, BuildResponseConfirmationRequest(List(), -1));
        return head;
    }

    task_t* lastTask = head;
    while(lastTask->nextTask != NULL)
    {
        lastTask = lastTask->nextTask;
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

    SendResponse(socketClient, BuildResponseConfirmationRequest(List(), -1));
    return head;
}

list_t* List()
{
    list_t* list = malloc(sizeof(list_t));

    char bodyEmptyTask[] = "The task is empty";
    list->bodyEmptyTask = calloc(strlen(bodyEmptyTask) + 1, sizeof(char));
    strcpy(list->bodyEmptyTask, bodyEmptyTask);

    char unknownFieldId[] = "unknown field id";
    list->unknownFieldId = calloc(strlen(unknownFieldId) + 1, sizeof(char));
    strcpy(list->unknownFieldId, unknownFieldId);

    char status200[] = "HTTP/1.1 200 OK \n";
    list->status200 = calloc(strlen(status200) + 1, sizeof(char));
    strcpy(list->status200, status200);

    char status201[] = "HTTP/1.1 201 Created \n";
    list->status201 = calloc(strlen(status201) + 1, sizeof(char));
    strcpy(list->status201, status201);

    char status400[] = "HTTP/1.1 400 \n";
    list->status400 = calloc(strlen(status400) + 1, sizeof(char));
    strcpy(list->status400, status400);

    char type[] = "Content-Type: application/json \n";
    list->type = calloc(strlen(type) + 1, sizeof(char));
    strcpy(list->type, type);

    char length[] = "Content-Length: ";
    list->length = calloc(strlen(length) + 1, sizeof(char));
    strcpy(list->length, length);

    char connection[] = "Connection: keep-alive \n";
    list->connection = calloc(strlen(connection) + 1, sizeof(char));
    strcpy(list->connection, connection);

    char errorValue[] = "You entered incorrect value";
    list->errorValue = calloc(strlen(errorValue) + 1, sizeof(char));
    strcpy(list->errorValue, errorValue);

    char error[] = "error";
    list->error = calloc(strlen(error) + 1, sizeof(char));
    strcpy(list->error, error);    

    char errorId[] = "You entered incorrect id";
    list->errorId = calloc(strlen(errorId) + 1, sizeof(char));
    strcpy(list->errorId, errorId);

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

int FindId(parse_t* parseRequest)
{
    char* body = IsolateBody(parseRequest);
    size_t sizeOfBody = strlen(body);

    int i = 0;
    while(i != sizeOfBody - 2)
    {
        if(body[i] == 'i' && body[i + 1] == 'd')
        {
            char* charId = ParseBody(i, parseRequest);
            int id = atoi(charId);
            free(charId);
            return id;
        }
        i++;
    }
    return -1;
}

char* FindName(parse_t* parseRequest)
{
    char* body = IsolateBody(parseRequest);
    size_t sizeOfBody = strlen(body);

    int i = 0;
    while(i != sizeOfBody - 4)
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
        size_t sizeOfErrorId = strlen(list->openedBracket) + strlen(list->quotes) + strlen(list->error) + strlen(list->quotes) 
    + strlen(list->colon) + strlen(list->quotes) + strlen(list->bodyEmptyTask) + strlen(list->quotes) + strlen(list->closedBracket) 
    + strlen(list->lineBreak);

    char* errorEmptyTask = calloc(sizeOfErrorId + 1, sizeof(char));

    strcat(errorEmptyTask, list->openedBracket);
    strcat(errorEmptyTask, list->quotes);
    strcat(errorEmptyTask, list->error);
    strcat(errorEmptyTask, list->quotes);
    strcat(errorEmptyTask, list->colon);
    strcat(errorEmptyTask, list->quotes);
    strcat(errorEmptyTask, list->bodyEmptyTask);
    strcat(errorEmptyTask, list->quotes);
    strcat(errorEmptyTask, list->closedBracket);
    strcat(errorEmptyTask, list->lineBreak);

    char* charSizeOfErrorId = calloc(strlen(errorEmptyTask), sizeof(char));

    sprintf(charSizeOfErrorId, "%ld", sizeOfErrorId);

    char* contentLength = calloc(strlen(list->length) + sizeOfErrorId + 1, sizeof(char));
    strcat(contentLength, list->length);
    strcat(contentLength, charSizeOfErrorId);

    size_t sizeOfResponse = strlen(list->status400) + strlen(list->type) + strlen(contentLength) + strlen(list->connection)
    + strlen(list->lineBreak) + strlen(errorEmptyTask);

    char* buildedResponse = calloc(sizeOfResponse + 1, sizeof(char));
    strcat(buildedResponse, list->status400);
    strcat(buildedResponse, list->type);
    strcat(buildedResponse, contentLength);
    strcat(buildedResponse, list->connection);
    strcat(buildedResponse, list->lineBreak);
    strcat(buildedResponse, errorEmptyTask);

    free(errorEmptyTask);
    free(charSizeOfErrorId);

    return buildedResponse;
}

char* BuildResponseConfirmationRequest(list_t* list, int id)
{
    if(id != -1)
    {
        char buffer[1024] = "\0";
        sprintf(buffer, "%d", id);

        char* charId = calloc(strlen(buffer) + 1, sizeof(char));
        strcpy(charId, buffer);

        size_t sizeOfResponseId = strlen(list->openedBracket) + strlen(list->quotes) + strlen(list->responseId) + strlen(list->quotes) 
        + strlen(list->colon) + strlen(list->quotes) + strlen(charId) + strlen(list->quotes) + strlen(list->closedBracket) + strlen(list->lineBreak);

        char* jsonResponseId = calloc(sizeOfResponseId + 1, sizeof(char));

        strcat(jsonResponseId, list->openedBracket);
        strcat(jsonResponseId, list->quotes);
        strcat(jsonResponseId, list->responseId);
        strcat(jsonResponseId, list->quotes);
        strcat(jsonResponseId, list->colon);
        strcat(jsonResponseId, list->quotes);
        strcat(jsonResponseId, charId);
        strcat(jsonResponseId, list->quotes);
        strcat(jsonResponseId, list->closedBracket);
        strcat(jsonResponseId, list->lineBreak);

        char* charSizeOfResponseId = calloc(strlen(jsonResponseId), sizeof(char));

        sprintf(charSizeOfResponseId, "%ld", sizeOfResponseId);

        char* contentLength = calloc(strlen(list->length) + strlen(jsonResponseId) + 1, sizeof(char));
        strcat(contentLength, list->length);
        strcat(contentLength, charSizeOfResponseId);

        size_t sizeOfResponse = strlen(list->status201) + strlen(list->type) + strlen(contentLength) + strlen(list->lineBreak) 
        + strlen(list->connection) + strlen(list->lineBreak) + strlen(jsonResponseId);

        char* buildedResponse = calloc(sizeOfResponse + 1, sizeof(char));
        strcat(buildedResponse, list->status201);
        strcat(buildedResponse, list->type);
        strcat(buildedResponse, contentLength);
        strcat(buildedResponse, list->lineBreak);

        strcat(buildedResponse, list->connection);
        strcat(buildedResponse, list->lineBreak);
        strcat(buildedResponse, jsonResponseId);

        free(jsonResponseId);
        free(charSizeOfResponseId);

        return buildedResponse;
    }



    char* contentLength = calloc(strlen(list->length) + strlen("0") + 1, sizeof(char));
    strcat(contentLength, list->length);
    strcat(contentLength, "0");

    size_t sizeOfResponse = strlen(list->status200) + strlen(list->type) + strlen(contentLength) + strlen(list->lineBreak) 
    + strlen(list->connection);

    char* buildedResponse = calloc(sizeOfResponse + 1, sizeof(char));
    strcat(buildedResponse, list->status200);
    strcat(buildedResponse, list->type);
    strcat(buildedResponse, contentLength);
    strcat(buildedResponse, list->lineBreak);
    strcat(buildedResponse, list->connection);

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

    size_t sizeOfResponse = strlen(list->status200) + strlen(list->type) + sizeOfContentLength + strlen(list->connection) 
    + strlen(list->lineBreak) + sizeOfNames + 1;

    char* buildedResponse = calloc(sizeOfResponse + 1, sizeof(char));

    strcat(buildedResponse, list->status200);
    strcat(buildedResponse, list->type);
    strcat(buildedResponse, ContentLength);
    strcat(buildedResponse, list->connection);
    strcat(buildedResponse, list->lineBreak);
    strcat(buildedResponse, names);

    FreeList(list);

    return buildedResponse;
}

char* BuildResponseErrorValue(list_t* list)
{
 size_t sizeOfErrorValue = strlen(list->openedBracket) + strlen(list->quotes) + strlen(list->error) + strlen(list->quotes) 
    + strlen(list->colon) + strlen(list->quotes) + strlen(list->errorValue) + strlen(list->quotes) +  strlen(list->closedBracket) 
    + strlen(list->lineBreak);

    char* errorValue = calloc(sizeOfErrorValue + 1, sizeof(char));
    strcat(errorValue, list->openedBracket);
    strcat(errorValue, list->quotes);
    strcat(errorValue, list->error);
    strcat(errorValue, list->quotes);
    strcat(errorValue, list->colon);
    strcat(errorValue, list->quotes);
    strcat(errorValue, list->errorValue);
    strcat(errorValue, list->quotes);
    strcat(errorValue, list->closedBracket);
    strcat(errorValue, list->lineBreak);


    char* charSizeOfErrorValue = calloc(sizeOfErrorValue, sizeof(char));
    sprintf(charSizeOfErrorValue, "%ld", sizeOfErrorValue);

    char* contentLength = calloc(strlen(list->length) + sizeOfErrorValue + 1, sizeof(char));
    strcat(contentLength, list->length);
    strcat(contentLength, charSizeOfErrorValue);

    size_t sizeOfResponse = strlen(list->status200) + strlen(list->type) + strlen(contentLength) 
    + strlen(list->lineBreak) + strlen(list->connection) + strlen(list->lineBreak) + sizeOfErrorValue + strlen(list->lineBreak);

    char* buildedResponse = calloc(sizeOfResponse + 1, sizeof(char));
    strcat(buildedResponse, list->status200);
    strcat(buildedResponse, list->type);
    strcat(buildedResponse, contentLength);
    strcat(buildedResponse, list->lineBreak);
    strcat(buildedResponse, list->connection);
    strcat(buildedResponse, list->lineBreak);
    strcat(buildedResponse, errorValue);
    strcat(buildedResponse, list->lineBreak);
    
    return buildedResponse;
}

char* BuildResponseErrorId(list_t* list)
{
    size_t sizeOfErrorId = strlen(list->openedBracket) + strlen(list->quotes) + strlen(list->error) + strlen(list->quotes) 
    + strlen(list->colon) + strlen(list->quotes) + strlen(list->errorId) + strlen(list->quotes) +  strlen(list->closedBracket) 
    + strlen(list->lineBreak);

    char* errorId = calloc(sizeOfErrorId + 1, sizeof(char));

    strcat(errorId, list->openedBracket);
    strcat(errorId, list->quotes);
    strcat(errorId, list->error);
    strcat(errorId, list->quotes);
    strcat(errorId, list->colon);
    strcat(errorId, list->quotes);
    strcat(errorId, list->unknownFieldId);
    strcat(errorId, list->quotes);
    strcat(errorId, list->closedBracket);
    strcat(errorId, list->lineBreak);

    char* charSizeOfErrorId = calloc(strlen(errorId), sizeof(char));

    sprintf(charSizeOfErrorId, "%ld", sizeOfErrorId);

    char* contentLength = calloc(strlen(list->length) + sizeOfErrorId + 1, sizeof(char));
    strcat(contentLength, list->length);
    strcat(contentLength, charSizeOfErrorId);

    size_t sizeOfResponse = strlen(list->status400) + strlen(list->type) + strlen(contentLength) + strlen(list->connection)
    + strlen(list->lineBreak) + strlen(errorId);

    char* buildedResponse = calloc(sizeOfResponse + 1, sizeof(char));
    strcat(buildedResponse, list->status400);
    strcat(buildedResponse, list->type);
    strcat(buildedResponse, contentLength);
    strcat(buildedResponse, list->connection);
    strcat(buildedResponse, list->lineBreak);
    strcat(buildedResponse, errorId);

    free(errorId);
    free(charSizeOfErrorId);

    return buildedResponse;
}

void FreeList(list_t* list)
{
    free(list->bodyEmptyTask);
    free(list->status200);
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

bool CheckErrorId(int socketClient, task_t* head, parse_t* parseRequest)
{
    bool isErrorExist = false;

    task_t* lastTask = head;
    while(lastTask->nextTask != NULL)
    {
        lastTask = lastTask->nextTask;
    }
    
    int id = FindId(parseRequest);
    
    if(id == -1)
    {
        SendResponse(socketClient, BuildResponseErrorId(List()));
        return isErrorExist = true;
    }

    if(id < head->id)
    {
        SendResponse(socketClient, BuildResponseErrorId(List()));
        return isErrorExist = true;
    }
    
    if(id > lastTask->id)
    {
        SendResponse(socketClient, BuildResponseErrorId(List()));
        return isErrorExist = true;
    }
    
    return isErrorExist;
}

bool CheckErrorName(int socketClient, parse_t* parseRequest)
{
    bool isErrorNameExist = false;

    char* name = FindName(parseRequest);

    if(name == NULL)
    {
        SendResponse(socketClient, BuildResponseErrorValue(List()));
        return isErrorNameExist = true;
    }

    return isErrorNameExist;
}