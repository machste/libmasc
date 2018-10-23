#include <netdb.h>
#include <arpa/inet.h>

#include <masc/socket.h>


Str *socket_gethostbyname(const char *hostname)
{
    Str *ip = NULL;
    struct hostent *host = gethostbyname(hostname);
    if (host != NULL) {
        ip = str_new_cstr(inet_ntoa(*((struct in_addr *)host->h_addr)));
    }
    return ip;
}
