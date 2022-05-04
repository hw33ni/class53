#include <sys/types.h>
#include <sys/uio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in_systm.h>

// These header files referenced SAS/C Socket Library
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <net/if.h>
#include <strings.h>

#include <string.h>
#include <stdlib.h> // stdtoul function
#include <unistd.h> // close function

#define SUCC 0
#define FAIL 1
#define ENDC 2

/*
function detect_quit:
    to detect terminate message "QUIT" in buffer
    dismiss buffer which length not equal to 4
    use strncmp to prevent BOF

return: ENDC(match terminate messege), SUCC(otherwise)
*/
int detect_quit(char* buf, int len)
{
    return len == 4 ? SUCC : (strncmp(buf, "QUIT", 4) == 0 ? ENDC : SUCC);
}