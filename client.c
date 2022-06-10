#include "header.h"


struct sockaddr_in addr;
char* nickname;

int sd; // client socket descriptor identification number

void usage()
{
    printf("./client [ip] [port] [nickname]\n");
    exit(0);
}


int initSocket(struct sockaddr_in *_addr, char* _ipString, char* _portString, char* _nickname)
{
    in_addr_t _ipNum = inet_addr(_ipString); // convert string to base type for internet address
    if(_ipNum == INADDR_NONE) return FAIL; // convert failure

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

    nickname = _nickname;
    // address family internet set to IPv4
    addr.sin_family = AF_INET; 
    addr.sin_addr.s_addr = _ipNum; 
    // convert port number from host order to network order (short type)
    addr.sin_port = htons(_portNum);

    return SUCC; // init success
}

void threadRecvFunc(void* arg)
{
    char rbuf[MAX_BUF] = {0};

    while(1)
    {
        if(recv(sd, rbuf, sizeof(rbuf), 0) < 0) continue;
        if(*rbuf != 0) puts(rbuf);
        MEMSET0(rbuf);
    }

}



void chatClient()
{
    // set buffer responsible for sending.
    char sbuf[MAX_BUF] = {0}; 

    pthread_t tid;
    pthread_create(&tid, NULL, (void*(*)(void*))threadRecvFunc, NULL);
    
    send(sd, nickname, strlen(nickname), 0);

    while(1)
    {
        if(sendMsg(sd, sbuf, sizeof(sbuf)) == SUCC){
            printf("%s is disconnected\n", nickname);
            break;
        }
        MEMSET0(sbuf);
    }
    pthread_cancel(tid);






    // while(1)
    // {
    //     // receive messege from terminal and save buffer length
    //     int sbufLen = read(0, sbuf, MAX_BUF); 
    //     if(sbufLen < 0) perror("client input error"); //read fail
    //     //send buffer to accepted server socket
    //     if(send(sd, sbuf, sbufLen, 0) == -1) perror("client send fail"); 
        
    //     sbuf[--sbufLen] = 0; 
        
    //     if(detect_quit(sbuf, sbufLen) == ENDC){ 
    //         printf("Disconnected\n");
    //         break; //end loop
    //     }

    //     // receive buffer from accepted server socket
    //     if(recv(sd, rbuf, MAX_BUF, 0) == -1) perror("client recv fail"); 
    //     int rbufLen = strlen(rbuf); //save buffer length
    //     //delete newline character and decrease buffer length parameter
    //     rbuf[--rbufLen] = 0; 
    //     printf("%s\n", rbuf); 
    //     // check buffer equal to "QUIT"
    //     if(detect_quit(rbuf, rbufLen) == ENDC){ 
    //         printf("Disconnected\n");    
    //         break; //end loop
    //     }

    //     memset(sbuf,0,MAX_BUF); // initialize send buffer
    //     memset(rbuf,0,MAX_BUF); // initialize receive buffer
    // }
    
}

int main(int argc, char** argv)
{

    // argument should be filename, IP and Port number
    if(argc != 4) usage(); 

    sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // create socket
    if(sd == -1) perror("socket create error"); // fail to create socket

    // initialize socket address and catch error
    if(initSocket(&addr, argv[1], argv[2], argv[3]) == FAIL) perror("socket init error"); 
    
    // make connection with server socket and catch error
    if(connect(sd, (struct sockaddr *) &addr, sizeof(addr)) != SUCC) perror("socket connect error"); 

    printf("%s is Connected\n", nickname);

    chatClient(); // chat sequence to server
    
    close(sd); // close socket after chat sequence
}