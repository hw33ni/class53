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
#include <stdlib.h>
#include <unistd.h>

#define SUCC 0
#define FAIL 1
#define ENDC 2

int detect_quit(char* buf, int len)
{
    
    return len != 4 ? SUCC : (strncmp(buf, "QUIT", 4) == 0 ? ENDC : SUCC);
}