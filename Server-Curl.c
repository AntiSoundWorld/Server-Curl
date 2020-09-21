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


int SetSocket();
parse_t* Recieve(int clientSocket, parse_t* parseRequest);
parse_t* ParseName(parse_t* parseRequest);
parse_t* ParseMethod(parse_t* parseRequest);
int MethodInterracive(parse_t* parseRequest);

int main()
{
    parse_t* parseRequest = malloc(sizeof(parse_t));
    MethodInterracive(ParseMethod(ParseName(Recieve(SetSocket(), parseRequest))));
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
    //printf("name = %s\n", name);     //show name

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
    //printf("method %s\n", method);   //show method

    return parseRequest;
}

int MethodInterracive(parse_t* ParseRequest)
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
