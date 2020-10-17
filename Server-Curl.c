#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>

typedef struct Parametrs
{
    char* request;
    char* method;
    char* isloatedParametrs;
    char* isolatedParametrId;
    char* isolatedParametrName;
    char* dataId;
    char* dataName;
}parametrs_t;

typedef struct Task
{
    int id;
    char* name;
    struct Task * nextTask;
}task_t;

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

void SetSocket();
void Lounch(int clientSocket, task_t* head, parametrs_t* parametrs);

void Recieve(int clientSocket, parametrs_t* parametrs);

void IsolateParametrs(parametrs_t* parametrs);
void IsolateParametrId(parametrs_t* parametrs);
void IsolateParametrName(parametrs_t* parametrs);

void ParseMethod(parametrs_t* parametr);
void ParseId(parametrs_t* parametrs);
void ParseName(parametrs_t* parametrs);

bool CheckSeparatorExist(parametrs_t* parametrs);

bool CheckParametrIdEqualesExist(parametrs_t* parametrs);
bool CheckLabelIdExist(parametrs_t* parametrs);
bool CheckParametrsIdExist(parametrs_t* parametrs);
bool CheckDataIdExist(parametrs_t* parametrs);
bool CheckIdExist(task_t* head, parametrs_t* parametrs);

bool CheckParametrNameEqualesExist(parametrs_t* parametrs);
bool CheckLabelNameExist(parametrs_t* parametrs);
bool CheckParametrsNameExist(parametrs_t* parametrs);
bool CheckDataNameExist(parametrs_t* parametrs);
bool CheckNameExist(parametrs_t* parametrs);


task_t* MethodInterractive(int socketClient, task_t* head, parametrs_t* parametrs);

char* BuildNames(char* buildNames, task_t* pointer);
char* BuildResponse(int id, char* names);
char* BuildResponseRead(list_t* list, char* names);
char* BuildResponseConfirmationRequest(list_t* list, int id);
char* BuildResponseEmptyTask(list_t* list);
char* BuildResponseErrorValue(list_t* list);
char* BuildResponseErrorId(list_t* list);

void SendResponse(int clientSocket, char* request);

task_t* Create (int socketClient, parametrs_t* parametrs, task_t* node);
void Read(int socketClient, task_t* head, parametrs_t* ParseRequest);
void Update(int clientSocket, task_t* head, parametrs_t* ParseRequest);
task_t* Delete(int socketClient, task_t* head, parametrs_t* parametrs);
void ReassigneId(task_t* head);

list_t* List();

void FreeParametrs(parametrs_t* parametrs);
void FreeList(list_t* list);

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
        parametrs_t* parametrs = (parametrs_t *)malloc(sizeof(parametrs_t));
        int clientSocket = accept(serverSocket, NULL, NULL);
        Lounch(clientSocket, head, parametrs);
        head = MethodInterractive(clientSocket, head, parametrs);

        FreeParametrs(parametrs);
    }
}

//--------------------------------------------------------------------------------------------------------------------------------------

void Recieve(int clientSocket, parametrs_t* parametrs)
{
    char buffer[1000] = "\0";
    recv(clientSocket, buffer, sizeof(buffer), 0);
    size_t sizeOfBuffer = strlen(buffer);

    parametrs->request = calloc(sizeOfBuffer + 1, sizeof(char));
    strcpy(parametrs->request, buffer);

    printf("Request:\n[%s]\n", parametrs->request);  //show full request
}

//==========================================================================================================================
//Parse

void ParseMethod(parametrs_t* parametr)
{
    char* request = parametr->request;
    size_t sizeOfRequest = strlen(request);

    char bufferOfMethod[256] = "\0";
    
    int i = 0;

    while(i < sizeOfRequest)
    {
        if(request[i] == ' ')
        {
            break;
        }

        bufferOfMethod[i] = request[i];
        i++;
    }

    parametr->method = calloc(strlen(bufferOfMethod) + 1, sizeof(char));
    strcat(parametr->method, bufferOfMethod);

    printf("method\n[%s]\n", parametr->method);
}

//--------------------------------------------------------------------------------------------------------------------------------------

void ParseId(parametrs_t* parametrs)
{
    if(CheckParametrsIdExist(parametrs) == false)
    {   
        parametrs->dataId = NULL;
        return;
    }

    char* isolatedParametrId = parametrs->isolatedParametrId;
    size_t sizeOfIsolatedParametrId = strlen(isolatedParametrId);

    int i = 0;
    while (i < sizeOfIsolatedParametrId)
    {
        if(isolatedParametrId[i] == '=')
        {
            break;
        }
        i++;
    }

    i++;

    char bufferOfParseId[256] = "\0";

    int j = 0;
    while(i < sizeOfIsolatedParametrId)
    {
        bufferOfParseId[j] = isolatedParametrId[i];
        i++;
        j++;
    }

    parametrs->dataId = calloc(strlen(bufferOfParseId) + 1, sizeof(char));
    strcat(parametrs->dataId, bufferOfParseId);

    printf("dataId:\n[%s]\n", parametrs->dataId);
}

//--------------------------------------------------------------------------------------------------------------------------------------

void ParseName(parametrs_t* parametrs)
{
    char* method = parametrs->method;

    char* isolatedParametrsName = parametrs->isolatedParametrName;
    size_t sizeOfIsolatedParametrsName = strlen(isolatedParametrsName);

    if(CheckParametrsNameExist(parametrs) == false)
    {
        parametrs->dataName = NULL;
        return;
    }

    int i = 0;

    while (i < sizeOfIsolatedParametrsName)
    {
        if(strcmp(method, "POST") == 0 && isolatedParametrsName[i] == ':' )
        {
            break;
        }

        if(strcmp(method, "PUT") == 0 && isolatedParametrsName[i] == '=' )
        {
            break;
        }

        i++;
    }

    i++;

    char bufferOfParseName[256] = "\0";

    int j = 0;

    while (i < sizeOfIsolatedParametrsName)
    {
        bufferOfParseName[j] = isolatedParametrsName[i];
        i++;
        j++;
    }
    
    parametrs->dataName = calloc(strlen(bufferOfParseName) + 1, sizeof(char));
    strcat(parametrs->dataName, bufferOfParseName);

    printf("dataName:\n[%s]\n", parametrs->dataName);
}

//=========================================================================================================================
//Isolate

void IsolateParametrs(parametrs_t* parametrs)
{
    char* request = parametrs->request;
    size_t sizeOfRequest = strlen(request);

    char* method = parametrs->method;

    int i = 0;

    i = 0;

    if(strcmp(method, "POST") == 0)
    {
        int j = 2;

        while (request[j] < sizeOfRequest - 1)
        {
            printf("%c[]\n", request[i]);

            if(request[i] == '\r' && request[i + 1] == '\n' && request[j] == '\r' && request[j + 1] == '\n')
            {
                break;
            }
       
            i++;
            j++;
        }

        i = j + 1;
    }

    if(strcmp(method, "PUT") == 0 || strcmp(method, "DELETE") == 0)
    {
        while(i < sizeOfRequest)
        {
            if(request[i] == '?')
            {
                break;
            }
            i++;
        }

        i++;
    }

    char bufferOfIsolatedParametrs[1024] = "\0";

    int j = 0;
    while(i < sizeOfRequest)
    {
        if(request[i] == '\0')
        {
            break;
        }
    
        bufferOfIsolatedParametrs[j] = request[i];

        j++;
        i++;
    }

    parametrs->isloatedParametrs = calloc(strlen(bufferOfIsolatedParametrs) + 1, sizeof(char));
    strcat(parametrs->isloatedParametrs, bufferOfIsolatedParametrs);

    printf("isloatedParametrs\n[%s]\n", parametrs->isloatedParametrs);
}

//--------------------------------------------------------------------------------------------------------------------------------------

void IsolateParametrId(parametrs_t* parametrs)
{
    char* method = parametrs->method;
    
    if(strcmp(method, "PUT") == 0)
    {
        if(CheckSeparatorExist(parametrs) == false)
        {
            parametrs->isolatedParametrId = NULL;
            return;
        }
    }

    char* isolatedParametrs = parametrs->isloatedParametrs;
    size_t sizeOfIsolatedParametrs = strlen(isolatedParametrs);

    char bufferOfIsolated[256] = "\0";


    int i = 0;


    while(i < sizeOfIsolatedParametrs)
    {
        if(isolatedParametrs[i] == '&')
        {
            break;
        }

        bufferOfIsolated[i] = isolatedParametrs[i];
        i++;
    }

    parametrs->isolatedParametrId = calloc(strlen(bufferOfIsolated) + 1, sizeof(char));
    strcat(parametrs->isolatedParametrId, bufferOfIsolated);

    printf("isolatedParametrId:\n[%s]\n", parametrs->isolatedParametrId);
}

//--------------------------------------------------------------------------------------------------------------------------------------

void IsolateParametrName(parametrs_t* parametrs)
{
    char* method = parametrs->method;

    if(strcmp(method, "PUT") == 0)
    {
        if(CheckSeparatorExist(parametrs) == false)
        {
            parametrs->isolatedParametrName = NULL;
            return;
        }
    }


    char* isolatedParametrs = parametrs->isloatedParametrs;
    size_t sizeOfIsolatedParametrs = strlen(isolatedParametrs);

    int i = 0;

    if(strcmp(method, "PUT") == 0)
    {
        while (i < sizeOfIsolatedParametrs)
        {
            if(isolatedParametrs[i] == '&')
            {
                break;
            }
            i++;
        }
    }

    i++;

    char bufferOfIsolatedParametrName[256] = "\0";

    int j = 0;

    while (i < sizeOfIsolatedParametrs)
    {
        if(isolatedParametrs[i] != '\"' && isolatedParametrs[i] != '{' && isolatedParametrs[i] != '}')
        {
            bufferOfIsolatedParametrName[j] = isolatedParametrs[i];
            j++;
        }
        i++;
    }
    
    parametrs->isolatedParametrName = calloc(strlen(bufferOfIsolatedParametrName) + 1, sizeof(char));
    strcat(parametrs->isolatedParametrName, bufferOfIsolatedParametrName);

    printf("isolatedParametrName\n[%s]\n", parametrs->isolatedParametrName);
}

//=========================================================================================================================
//Check

bool CheckSeparatorExist(parametrs_t* parametrs)
{
    bool isAmpersandExist = false;

    char* isolatedParametrs = parametrs->isloatedParametrs;
    size_t sizeOfIsolatedParametrs = strlen(isolatedParametrs);

    int i = 0;

    while(i < sizeOfIsolatedParametrs)
    {
        if(isolatedParametrs[i] == '&')
        {
            isAmpersandExist = true;
        }
        i++;
    }

    return isAmpersandExist;
}

//--------------------------------------------------------------------------------------------------------------------------------------

bool CheckLabelIdExist(parametrs_t* parametrs)
{
    bool isLabelIdExist = false;

    char* isolatedParametrId = parametrs->isolatedParametrId;
    size_t sizeOfIsolatedParametrId = strlen(isolatedParametrId);

    char bufferLabelId[256] = "\0";

    int i = 0;

    while(i < sizeOfIsolatedParametrId)
    {
        if(isolatedParametrId[i] == '=')
        {
            break;
        }
        bufferLabelId[i] = isolatedParametrId[i];
        i++;
    }
    
    if(strcmp(bufferLabelId, "id") == 0)
    {
        isLabelIdExist = true;
    }

    printf("isLabelIdExist [%d]\n", isLabelIdExist);
    return isLabelIdExist;
}

//--------------------------------------------------------------------------------------------------------------------------------------

bool CheckParametrIdEqualesExist(parametrs_t* parametrs)
{
    bool isParametrIdEqualesExist = false;

    char* isolatedParametrId = parametrs->isolatedParametrId;

    if(isolatedParametrId == NULL)
    {
        return false;
    }
    
    if(isolatedParametrId[2] == '=')
    {
        isParametrIdEqualesExist = true;
    }

    return isParametrIdEqualesExist;
}

//--------------------------------------------------------------------------------------------------------------------------------------

bool CheckDataIdExist(parametrs_t* parametrs)
{
    bool isDataIdExist = false;

    char* isolatedParametrId = parametrs->isolatedParametrId;

    if(isolatedParametrId[3] != '\0')
    {
        isDataIdExist = true;
    }

    printf("isDataIdExist[%d]\n", isDataIdExist);
    return isDataIdExist;
}

//--------------------------------------------------------------------------------------------------------------------------------------

bool CheckIdExist(task_t* head, parametrs_t* parametrs)
{
    bool isIdExist = false;

    if(CheckParametrsIdExist(parametrs) == false || head == NULL)
    {
        return isIdExist;
    }

    int id = atoi(parametrs->dataId);

    task_t* lastTask = head;

    while (lastTask->nextTask != NULL)
    {
       lastTask = lastTask->nextTask;
    }
    
    if(id >= head->id && id <= lastTask->id)
    {
        isIdExist = true;
    }

    printf("isIdExist[%d]\n", isIdExist);
    return isIdExist;
}

//--------------------------------------------------------------------------------------------------------------------------------------

bool CheckParametrsIdExist(parametrs_t* parametrs)
{
    bool isParametrsIdExist = false;

    bool isLabelIdExist = CheckLabelIdExist(parametrs);
    bool isParametrIdEqualesExist = CheckParametrIdEqualesExist(parametrs);
    bool isDataIdExist = CheckDataIdExist(parametrs);

    if(isLabelIdExist == true && isParametrIdEqualesExist == true && isDataIdExist == true)
    {
        isParametrsIdExist = true;
    }

    printf("isParametrsIdExist [%d]\n", isParametrsIdExist);
    return isParametrsIdExist;
}

//--------------------------------------------------------------------------------------------------------------------------------------

bool CheckParametrNameEqualesExist(parametrs_t* parametrs)
{
    bool isParametrNameEqualesExist = false;

    char* isolatedParametrName = parametrs->isolatedParametrName;

    if(isolatedParametrName[4] == '=' || isolatedParametrName[4] == ':')
    {
        isParametrNameEqualesExist = true;
    }

    printf("isParametrNameEqualesExist[%d]\n", isParametrNameEqualesExist);

    return isParametrNameEqualesExist;
}


//--------------------------------------------------------------------------------------------------------------------------------------


bool CheckLabelNameExist(parametrs_t* parametrs)
{
    bool isLabelNameExist = false;

    char* isolatedParametrName = parametrs->isolatedParametrName;
    size_t sizeOfIsolatedParametrName = strlen(isolatedParametrName);

    char bufferOfLabelName[256] = "\0";

    int i = 0;
    while(i < sizeOfIsolatedParametrName)
    {
        if(isolatedParametrName[i] == '=' || isolatedParametrName[i] == ':')
        {
            break;
        }

        bufferOfLabelName[i] = isolatedParametrName[i];
        i++;
    }

    if(strcmp(bufferOfLabelName, "name") == 0)
    {
        isLabelNameExist = true; 
    }

    printf("isLabelNameExist[%d]\n", isLabelNameExist);
    return isLabelNameExist;
}

//--------------------------------------------------------------------------------------------------------------------------------------

bool CheckDataNameExist(parametrs_t* parametrs)
{
    bool isDataNameExist = false;

    char* isolatedParametrName = parametrs->isolatedParametrName;
    
    if(isolatedParametrName[5] != '\0' || isolatedParametrName[5] != ' ')
    {
        isDataNameExist = true;
    }


    return isDataNameExist;
}

//--------------------------------------------------------------------------------------------------------------------------------------

bool CheckParametrsNameExist(parametrs_t* parametrs)
{
    bool isParametrsNameExist = false;

    bool isLabelNameExist = CheckLabelNameExist(parametrs);
    bool isParametrNameParametrEqualseExist = CheckParametrNameEqualesExist(parametrs);
    bool isDataNameExist = CheckDataNameExist(parametrs);

    if(isLabelNameExist == true && isParametrNameParametrEqualseExist == true && isDataNameExist == true)
    {
       isParametrsNameExist = true;
    }

    printf("isParametrsNameExist[%d]\n", isParametrsNameExist);
    return isParametrsNameExist;
}

//--------------------------------------------------------------------------------------------------------------------------------------

bool CheckNameExist(parametrs_t* parametrs)
{
    bool isNameExist = false;

    if(CheckParametrsNameExist(parametrs) == false)
    {
        return isNameExist;
    }

    char* name = parametrs->dataName;

    if(name[0] != '\0')
    {
        isNameExist = true;
    }

    printf("isNameExist:\n[%d]\n", isNameExist);

    return isNameExist;
}

//=========================================================================================================================
//Interractive

task_t* MethodInterractive(int socketClient, task_t* head, parametrs_t* parametrs)
{
    char* method = parametrs->method;
    
    if(strcmp(method, "GET") == 0)
    {
        Read(socketClient, head, parametrs);
    }
    if(strcmp(method, "POST") == 0)
    {
        return Create(socketClient, parametrs, head);
    }
    if(strcmp(method, "PUT") == 0)
    {
        Update(socketClient, head, parametrs);
    }
    if(strcmp(method, "DELETE") == 0)
    {
        return Delete(socketClient, head, parametrs);
    }
    return head;
}

//=========================================================================================================================
//CRUD

task_t* Create(int socketClient, parametrs_t* parametrs, task_t* head)
{
    if(CheckNameExist(parametrs) == false)
    {
        SendResponse(socketClient, BuildResponseErrorValue(List()));
        return head;
    }

    printf("parametrs->dataName <%s>", parametrs->dataName);

    char* name = parametrs->dataName;

    if(head == NULL)
    {
        head = (task_t*)malloc(sizeof(task_t));
        head->id = 0;

        head->name = calloc(strlen(name) + 1, sizeof(char));
        strcpy(head->name, name);

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

//-----------------------------------------------------------------------------------------------------------------------------------

void Read(int socketClient, task_t* head, parametrs_t* parametrs)
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

    free(buildedNames);
    //printf("\n\nbuildedNames\n %s\n\n", buildedNames);
}

//-----------------------------------------------------------------------------------------------------------------------------------

void Update(int socketClient, task_t* head, parametrs_t* parametrs)
{
    if(CheckIdExist(head, parametrs) == false)
    {
        SendResponse(socketClient, BuildResponseErrorId(List()));
        return;
    }

    if(CheckNameExist(parametrs) == false)
    {
        SendResponse(socketClient, BuildResponseErrorValue(List()));
        return;
    }

    int id = atoi(parametrs->dataId);
    
    char* newName = parametrs->dataName;

    task_t* pointer = head;

    while(pointer->id != id)
    {
        pointer = pointer->nextTask;
    }
    free(pointer->name);
    
    pointer->name = calloc(strlen(newName) + 1, sizeof(char));
    strcat(pointer->name, newName);
    
    SendResponse(socketClient, BuildResponseConfirmationRequest(List(), -1));
}

//-----------------------------------------------------------------------------------------------------------------------------------

task_t* Delete(int socketClient, task_t* head, parametrs_t* parametrs)
{
    if(CheckIdExist(head, parametrs) == false)
    {
        SendResponse(socketClient, BuildResponseErrorId(List()));
        return head;
    }


    int id = atoi(parametrs->dataId);

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

//-----------------------------------------------------------------------------------------------------------------------------------

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

//=========================================================================================================================
//Send

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

//=========================================================================================================================
//Builds

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

//-----------------------------------------------------------------------------------------------------------------------------------

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

    char* contentLength = calloc(strlen(list->length) + sizeOfErrorId + strlen(list->lineBreak) + 1, sizeof(char));
    strcat(contentLength, list->length);
    strcat(contentLength, charSizeOfErrorId);
    strcat(contentLength, list->lineBreak);

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
    FreeList(list);

    return buildedResponse;
}

//-----------------------------------------------------------------------------------------------------------------------------------

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
        free(charId);
        free(contentLength);
        FreeList(list);

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

    free(contentLength);
    FreeList(list);

    return buildedResponse;
}

//-----------------------------------------------------------------------------------------------------------------------------------

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
    
    free(ContentLength);
    free(charSizeOfNames);
    FreeList(list);

    return buildedResponse;
}

//-----------------------------------------------------------------------------------------------------------------------------------

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
    
    FreeList(list);

    return buildedResponse;
}

//-----------------------------------------------------------------------------------------------------------------------------------

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

    size_t sizeOfResponse = strlen(list->status400) + strlen(list->type) + strlen(contentLength) + strlen(list->lineBreak) + strlen(list->connection)
    + strlen(list->lineBreak) + strlen(errorId);

    char* buildedResponse = calloc(sizeOfResponse + 1, sizeof(char));
    strcat(buildedResponse, list->status400);
    strcat(buildedResponse, list->type);
    strcat(buildedResponse, contentLength);
    strcat(buildedResponse, list->lineBreak);
    strcat(buildedResponse, list->connection);
    strcat(buildedResponse, list->lineBreak);
    strcat(buildedResponse, errorId);

    free(errorId);
    free(charSizeOfErrorId);
    FreeList(list);

    return buildedResponse;
}

//=========================================================================================================================

void Lounch(int clientSocket, task_t* head, parametrs_t* parametrs)
{
    Recieve(clientSocket, parametrs);
    ParseMethod(parametrs);
    IsolateParametrs(parametrs);

    char* method = parametrs->method;

    if(strcmp(method, "POST") == 0)
    {
        IsolateParametrName(parametrs);
        ParseName(parametrs);
    }

    if(strcmp(method, "PUT") == 0)
    {
        IsolateParametrName(parametrs);
        ParseName(parametrs);
        IsolateParametrId(parametrs);
        ParseId(parametrs);
    }

    if(strcmp(method, "DELETE") == 0)
    {
        IsolateParametrId(parametrs);
        ParseId(parametrs);
    }

}

//=========================================================================================================================
//List

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
//=========================================================================================================================
//Free

void FreeParametrs(parametrs_t* parametrs)
{
    char* method = parametrs->method;

    if(strcmp(method, "POST") == 0)
    {
        free(parametrs->dataName);
        free(parametrs->isolatedParametrName);
    }

    if(strcmp(method, "PUT") == 0)
    {
        free(parametrs->dataName);
        free(parametrs->isolatedParametrName);

        free(parametrs->isolatedParametrId);
        free(parametrs->dataId);
    }

    if(strcmp(method, "DELETE") == 0)
    {
        free(parametrs->isolatedParametrId);
        free(parametrs->dataId);
    }

    free(parametrs->isloatedParametrs);
    free(parametrs->method);
    free(parametrs->request);
}

//-----------------------------------------------------------------------------------------------------------------------------------

void FreeList(list_t* list)
{
    free(list->bodyEmptyTask);
    free(list->status200);
    free(list->status201);
    free(list->status400);
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
    free(list->error);
    free(list->errorId);
    free(list->errorValue);
    free(list->unknownFieldId);
    free(list);
}