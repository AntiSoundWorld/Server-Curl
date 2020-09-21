#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>

typedef struct Parse
{
    char* clientRequest;
    char* name;
    char* method;
    int id;
}parse_t;
void Request();
parse_t* ParseMethod(parse_t* request);

int main()
{
    Request();
}
void Request()
{
    char request[] =
    "POST / HTTP/1.1"
    "Host: 127.0.0.1:8080"
    "User-Agent: curl/7.68.0"
    "Accept: */*"
    "Content-Length: 8"
    "\n"
    "{Dima}";
    parse_t* parse = malloc(sizeof(parse_t));
    parse->clientRequest = request;

    ParseMethod(parse);
}
parse_t* ParseMethod(parse_t* parseRequest)
{
    
    char* space = strchr(parseRequest->clientRequest, ' ');
    char buffer[25];

    for(int i = 0; parseRequest->clientRequest[i] != space[0]; i++)
    {
        buffer[i] = parseRequest->clientRequest[i];
    }

    size_t sizeOfMethod = strlen(buffer);

    char method[sizeOfMethod];
    strcpy(method, buffer);
    parseRequest->method = method;

    printf("method - [%s]\n", parseRequest->method);

    return parseRequest;
}