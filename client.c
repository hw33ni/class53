#include "socket.h"

#define MAX_BUF 256

struct sockaddr_in addr;

int sd, sdConnect;

void usage()
{
    printf("./client [ip] [port]\n");
    exit(0);
}

int initSocket(struct sockaddr_in *_addr, char* _ipString, char* _portString)
{
    in_addr_t _ipNum = inet_addr(_ipString);
    if(_ipNum == INADDR_NONE) return FAIL;

    char* EndPtr = 0;
    int16_t _portNum = strtoul(_portString, &EndPtr,10);
    if(_portString == EndPtr || *EndPtr != 0) return FAIL;


    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = _ipNum;
    addr.sin_port = htons(_portNum);

    return SUCC;
}

void chatClient()
{
    char sbuf[MAX_BUF] = {0}, rbuf[MAX_BUF] = {0};

    while(1)
    {
        int sbufLen = read(0, sbuf, MAX_BUF);
        if(sbufLen == -1) perror("client input error");
        if(send(sd, sbuf, sbufLen, 0) == -1) perror("client send fail");
        sbuf[--sbufLen] = 0;
        if(detect_quit(sbuf, sbufLen) == ENDC){
            printf("Disconnected\n");    
            break;
        }

        if(recv(sd, rbuf, MAX_BUF, 0) == -1) perror("client recv fail");
        int rbufLen = strlen(rbuf);
        rbuf[--rbufLen] = 0;
        printf("%s\n", rbuf);
        if(detect_quit(rbuf, rbufLen) == ENDC){
            printf("Disconnected\n");    
            break;
        }

        memset(rbuf,0,MAX_BUF);
        memset(sbuf,0,MAX_BUF);
    }
    
}

int main(int argc, char** argv)
{

    if(argc != 3) usage();

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd == -1) perror("socket def error");

    if(initSocket(&addr, argv[1], argv[2]) == FAIL) perror("socket init error");

    sdConnect = connect(sd, (struct sockaddr *) &addr, sizeof(addr));
    if(sdConnect == -1) perror("socket connect error");

    printf("Connected\n");

    chatClient();
    
    close(sd);
}