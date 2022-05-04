#include "socket.h"

#define MAX_BUF 256 //set MAX size of buffer

struct sockaddr_in addr;
socklen_t addrLen;

// descripter identification number of server socket, and accepted server socket
int sd, sdaccept; 

void usage()
{
    printf("./server [port]\n");
    exit(0);
}

/*
function initSocket:
    to initialize server's sockaddr_in
    convert string to num for port number

return: SUCC(0), FAIL(1)
*/
int initSocket(struct sockaddr_in *_addr, char* _portString)
{
    char* endptr = 0; // check parameter of strtoul
    errno = 0; // check parameter of strtoul
    
    // convert string to unsigned int
    int16_t _portNum = strtoul(_portString, &endptr, 10); 
    // check for various errors of strtoul function
    if ((errno == ERANGE && (_portNum == LONG_MAX || _portNum == LONG_MIN)) 
    || (errno != 0 && _portNum == 0) || endptr == _portString) { 
        perror("stotoul fail");
        return FAIL;
    } // ref. https://linux.die.net/man/3/strtol

    // address family internet set to IPv4
    addr.sin_family = AF_INET; 
    // convert arbitary IP(0.0.0.0) from host order to network order (long type)
    addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    // convert port number from host order to network order (short type)
    addr.sin_port = htons(_portNum);

    return SUCC; // init success
}

/*
function chatClient:
    chatting sequence of accepted server socket
*/
void chatServer()
{
    // set two buffer responsible for receiving and sending.
    char rbuf[MAX_BUF] = {0}, sbuf[MAX_BUF] = {0};

    while(1)
    {
        // receive buffer from accepted server socket
        if(recv(sdaccept, rbuf, MAX_BUF, 0) == -1) perror("server recv fail");
        int rbufLen = strlen(rbuf); //save buffer length
        //delete newline character and decrease buffer length parameter
        rbuf[--rbufLen] = 0;
        printf("%s\n", rbuf);
        // check buffer equal to "QUIT"
        if(detect_quit(rbuf, rbufLen) == ENDC){
            printf("Disconnected\n");    
            break; //end loop
        }

        // receive messege from terminal and save buffer length
        int sbufLen = read(0, sbuf, MAX_BUF);
        if(sbufLen == -1) perror("client input error"); // read fail
        //send buffer to accepted server socket
        if(send(sdaccept, sbuf, sbufLen, 0) == -1) perror("server send fail");
        // delete newline character and decrease buffer length parameter
        sbuf[--sbufLen] = 0;
        // check buffer equal to "QUIT"
        if(detect_quit(sbuf, sbufLen) == ENDC){
            printf("Disconnected\n");    
            break; // end loop
        }

        memset(rbuf,0,MAX_BUF); // initialize receive buffer
        memset(sbuf,0,MAX_BUF); // initialize send buffer
    }
    
}

int main(int argc, char** argv)
{

    // argument should be filename and port number
    if(argc != 2) usage(); 

    sd = socket(AF_INET, SOCK_STREAM, 0); // create socket
    if(sd == -1) perror("socket def error"); // fail to create socket

    // initialize socket address and catch error
    if(initSocket(&addr, argv[1]) == FAIL) perror("socket init error");

    // bind socket with IP and port number, and catch error
    if(bind(sd, (struct sockaddr*) &addr, sizeof(addr)) < 0) perror("socket bind error");

    // make server socket to listening mode to response client's connect request
    if(listen(sd, 1) < 0) perror("socket listen error");

    //accept client's request and create established socket that transmit to client
    sdaccept = accept(sd, (struct sockaddr*) &addr, (socklen_t*) &addrLen);
    //fail to accept
    if(sdaccept == -1) perror("socket accept error");

    printf("Connection From %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    chatServer(); // chat sequence to client
    
    close(sd); // close socket after chat sequence
}