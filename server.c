#include "header.h"

#define MAX_CLIENT 5
#define MAX_SERBUF 100

struct sockaddr_in saddr, caddr;
socklen_t caddrLen;

// descripter identification number of server socket, and accepted server socket
int sd, sdaccept, clientNum = 0; 
int clientArr[MAX_CLIENT] = {0};

pthread_t tid[MAX_CLIENT];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


// define struct to manage client argument
typedef struct _threadArg
{
    int idx;
    struct sockaddr_in caddr;
}clientArg;


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
    saddr.sin_family = AF_INET; 
    // convert arbitary IP(0.0.0.0) from host order to network order (long type)
    saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    // convert port number from host order to network order (short type)
    saddr.sin_port = htons(_portNum);

    return SUCC; // init success
}

//send message to connected client
void broadcast(int index, char* str, int len)
{
    pthread_mutex_lock(&mutex);
    for(int i = 0;i < MAX_CLIENT;i++){
        if(i == index || clientArr[i] == -1) continue; // not connected
        send(clientArr[i], str, len, 0);
    }
    pthread_mutex_unlock(&mutex);// critical error fixed.........
}

void disconnectClient(int index)
{
    pthread_mutex_lock(&mutex);
    close(clientArr[index]);
    clientArr[index] = -1;
    clientNum--;
    pthread_mutex_unlock(&mutex);
    return;
}


void serverThread(void* arg)
{
    char clientNickname[MAX_BUF] = {0};
    //send from server, receive from client buffer 
    char sbuf[MAX_BUF] = {0}, cbuf[MAX_BUF] = {0}; 

    clientArg ca = *(clientArg*)arg;
    
    printf("Connection From %s:%d\n", inet_ntoa(ca.caddr.sin_addr), ntohs(ca.caddr.sin_port));
    recv(clientArr[ca.idx], clientNickname, MAX_BUF, 0); // recv nickname
    snprintf(sbuf, MAX_BUF, "%s is connected", clientNickname);
    broadcast(ca.idx, sbuf, strlen(sbuf));
    puts(sbuf);

    int flag = SUCC;
    while(1)
    {
        MEMSET0(sbuf); // make server message
        MEMSET0(cbuf); // recv client's message

        if(recvMsg(clientArr[ca.idx], cbuf, MAX_BUF) == ENDC){
            flag = ENDC;
            snprintf(sbuf, MAX_BUF, "%s is disconnected", clientNickname);
        } else {
            snprintf(sbuf, MAX_BUF, "%s: %s", clientNickname, cbuf);
        }

        broadcast(ca.idx, sbuf, sizeof(sbuf));
        puts(sbuf);
        if(flag == ENDC) {
            disconnectClient(ca.idx);
            break;
        }
    }
}



int main(int argc, char** argv)
{

    MEMSETn(clientArr);

    // argument should be filename and port number
    if(argc != 2) usage(); 

    sd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // create socket
    if(sd == -1) perror("socket def error"); // fail to create socket

    // initialize socket address and catch error
    if(initSocket(&saddr, argv[1]) == FAIL) perror("socket init error");

    // saddr.sin_family = AF_INET;
    // uint16_t pn = atoi(argv[1]);
	// saddr.sin_port = htons(pn);
	// saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // bind socket with IP and port number, and catch error
    if(bind(sd, (struct sockaddr*) &saddr, sizeof(saddr)) < 0) perror("socket bind error");

    // make server socket to listening mode to response client's connect request
    if(listen(sd, MAX_CLIENT) < 0) perror("socket listen error");

    clientArg ca;
    caddrLen = sizeof(caddr);
    while(1)
    {
        pthread_mutex_lock(&mutex);
        if(clientNum < MAX_CLIENT) clientNum++;
        else {
            pthread_mutex_unlock(&mutex);
            continue;
        }
        pthread_mutex_unlock(&mutex);

        sdaccept = accept(sd, (struct sockaddr*) &caddr, &caddrLen);

        pthread_mutex_lock(&mutex);
        if(sdaccept == -1) {
            clientNum--;
            pthread_mutex_unlock(&mutex);
            continue;
        }
        
        for(int i = 0;i < MAX_CLIENT;i++) {
            if(clientArr[i] == -1) {
                ca.idx = i;
                clientArr[i] = sdaccept;
                break;
            }
        }
        pthread_mutex_unlock(&mutex);
        ca.caddr = caddr;
        pthread_create(tid+ca.idx, NULL, (void*(*)(void*))serverThread, (void*)&ca);
    }
    return 0;
    
    close(sd); // close socket after chat sequence
}