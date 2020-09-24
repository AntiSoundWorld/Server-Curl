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
parse_t* ParseName(parse_t* parseRequest);
parse_t* ParseMethod(parse_t* parseRequest);
int MethodID(parse_t* parseRequest);
task_t* Post (int socketClient, parse_t* parseRequest, task_t* node);
void MethodInterractive(int socketClient, task_t* head, parse_t* parseRequest);
void Get(int socketClient, task_t* head);
char* BuildResponse(int id);
void RequestInterractive(int clientSocket, int id, char* request);

int main()
{


    SetSocket();
   
}

void SetSocket()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in setConnect;
    setConnect.sin_family = AF_INET;
    setConnect.sin_port = ntohs(8080);
    setConnect.sin_addr.s_addr = inet_addr("127.0.0.1");

    bind(serverSocket, (const struct sockaddr*) &setConnect, sizeof(setConnect));
    listen(serverSocket, 1);


    task_t* head = NULL;

    parse_t* parseRequest = malloc(sizeof(parse_t));
    int clientSocket = accept(serverSocket, NULL, NULL);

    parseRequest = ParseName(ParseMethod(Recieve(clientSocket, parseRequest)));
    MethodInterractive(clientSocket, head, parseRequest);
    free(parseRequest);
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
    parseRequest->method = malloc(sizeof(size));
    strcpy(parseRequest->method, buffer);

    //printf("method %s\n", parseRequest->method);   //show method

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
        parseRequest->name = malloc(sizeof(size));
        strcpy(parseRequest->name, buffer);
        //printf("name = %s\n", parseRequest->name);     //show name
    }
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

task_t* Post (int socketClient, parse_t* parseRequest, task_t* head)
{
    if(head == NULL)
    {
        printf("I am in the POST");
        head = malloc(sizeof(task_t));
        head->id = 0;

        head->name = parseRequest->name;
        free(parseRequest);

        head->nextTask = NULL;
        
        RequestInterractive(socketClient, 0, BuildResponse(1));

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

    pointer->nextTask = NULL;

    return head;
}

void Get(int socketClient, task_t* head)
{
    if(head == NULL)
    {   
        RequestInterractive(socketClient, 0, BuildResponse(0));
        return;
    }
}

void MethodInterractive(int socketClient, task_t* head, parse_t* parseRequest)
{
    switch (MethodID(parseRequest))
    {
        case 0:
            Get(socketClient, head);
            break;
        case 1:
            Post(socketClient, parseRequest, head);
            break;
    }
}

char* BuildResponse(int id)
{
    char bodyEmptyTask[] = "The task is empty";
    size_t sizeOfEmptyTask = strlen(bodyEmptyTask);
    
    char status[] = "HTTP/1.1 200 OK \n";

    char type[] = "Content-Type: text \n";

    char length[] = "Content-Length: ";

    char lineBreak[] = "\n";

    if(id == 0)
    {
        char* charSizeEmptyTask = malloc(sizeof(char));
        sprintf(charSizeEmptyTask, "%ld", sizeOfEmptyTask);

        strcat(length, charSizeEmptyTask);
        strcat(length, lineBreak);
        
        char* buildedResponse = malloc(sizeof(char));

        strcat(buildedResponse, status);
        strcat(buildedResponse, type);
        strcat(buildedResponse, length);
        strcat(buildedResponse, lineBreak);
        strcat(buildedResponse, bodyEmptyTask);

        //printf("%s \n", buildedResponse); //show full response
        free(charSizeEmptyTask);
        return buildedResponse;
    }
    if(id == 1)
    {
        char* buildedResponse = malloc(sizeof(char));
        strcat(buildedResponse, status);
        return buildedResponse;
    }
    return NULL;
}

void RequestInterractive(int clientSocket, int id, char* buildedResponse)
{   
    size_t sizeOfBuildedResponse = strlen(buildedResponse);
    char response[sizeOfBuildedResponse];

    unsigned i = 0;
    while(i != sizeOfBuildedResponse)
    {
        response[i] = buildedResponse[i];
        i++;
    }
    printf("%s \n", response);

    switch (id)
    {
    case 0:
        send(clientSocket, response, sizeof(response), 0);
        break;
    
    default:
        break;
    }
}