#include "socket.h"

#define MAX_BUF 256

struct sockaddr_in addr;
socklen_t addrLen;
int sdaccept;

void usage()
{
    printf("./server [port]\n");
    exit(0);
}

int initSocket(struct sockaddr_in *_addr, char* _portString)
{
    char* EndPtr = 0;
    int16_t _portNum = strtoul(_portString, &EndPtr,10);
    if(_portString == EndPtr || *EndPtr != 0) return FAIL;

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(_portNum);

    return SUCC;
}

void chatServer()
{
    char rbuf[MAX_BUF] = {0}, sbuf[MAX_BUF] = {0};

    while(1)
    {
        if(recv(sdaccept, rbuf, MAX_BUF, 0) == -1) perror("server recv fail");
        int rbufLen = strlen(rbuf);
        rbuf[--rbufLen] = 0;
        printf("%s\n", rbuf);
        if(detect_quit(rbuf, rbufLen) == ENDC){
            printf("Disconnected\n");    
            break;
        }

        int sbufLen = read(0, sbuf, MAX_BUF);
        if(sbufLen == -1) perror("client input error");
        if(send(sdaccept, sbuf, sbufLen, 0) == -1) perror("server send fail");
        sbuf[--sbufLen] = 0;
        if(detect_quit(sbuf, sbufLen) == ENDC){
            printf("Disconnected\n");    
            break;
        }

        memset(rbuf,0,MAX_BUF);
        memset(rbuf,0,MAX_BUF);
    }
    
}

int main(int argc, char** argv)
{

    if(argc != 2) usage();

    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd == -1) perror("socket def error");

    if(initSocket(&addr, argv[1]) == FAIL) perror("socket init error");

    if(bind(sd, (struct sockaddr*) &addr, sizeof(addr)) < 0) perror("socket bind error");

    if(listen(sd, 1) < 0) perror("socket listen error");

    sdaccept = accept(sd, (struct sockaddr*) &addr, (socklen_t*) &addrLen);
    if(sdaccept == -1) perror("socket accept error");

    printf("Connection From %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    chatServer();
    
    close(sd);
}