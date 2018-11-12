#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <masc/socket.h>


Socket *socket_new(int domain, int type, int protocol)
{
    Socket *self = malloc(sizeof(Socket));
    socket_init(self, domain, type, protocol);
    return self;
}

void socket_init(Socket *self, int domain, int type, int protocol)
{
    object_init(self, SocketCls);
    self->fd = socket(domain, type, protocol);
}

static void _vinit(Socket *self, va_list va)
{
    int domain = va_arg(va, int);
    int type = va_arg(va, int);
    int protocol = va_arg(va, int);
    socket_init(self, domain, type, protocol);
}

void socket_destroy(Socket *self)
{
    close(self);
}

void socket_delete(Socket *self)
{
    socket_destroy(self);
    free(self);
}

Str *socket_gethostbyname(const char *hostname)
{
    Str *ip = NULL;
    struct hostent *host = gethostbyname(hostname);
    if (host != NULL) {
        ip = str_new_cstr(inet_ntoa(*((struct in_addr *)host->h_addr)));
    }
    return ip;
}

bool socket_connect(Socket *self, const char *ip, int port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;        
    addr.sin_port = htons(port);    
    if (!inet_aton(ip, &addr.sin_addr)) {
        errno = EINVAL;
        return false;
    }
    return connect(self->fd, (struct sockaddr *)&addr, sizeof(addr)) == 0;
}

bool socket_bind(Socket *self, const char *ip, int port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;        
    addr.sin_port = htons(port);    
    if (!inet_aton(ip, &addr.sin_addr)) {
        errno = EINVAL;
        return false;
    }
    return bind(self->fd, (struct sockaddr *)&addr, sizeof(addr)) == 0;
}

bool socket_listen(Socket *self, int backlog)
{
    return listen(self->fd, backlog) == 0;
}

Socket *socket_accept(Socket *self, struct sockaddr *addr, socklen_t *addrlen)
{
    Socket *sock = NULL;
    int fd = accept(self->fd, addr, addrlen);
    if (fd >= 0) {
        sock = malloc(sizeof(Socket));
        object_init(sock, SocketCls);
        sock->fd = fd;
    }
    return sock;
}

int socket_getsockopt(Socket *self, int level, int optname, void *optval,
        socklen_t *optlen)
{
    return getsockopt(self->fd, level, optname, optval, optlen);
}

int socket_setsockopt(Socket *self, int level, int optname, void *optval,
        socklen_t optlen)
{
    return setsockopt(self->fd, level, optname, optval, optlen);
}


static io_class _SocketCls = {
    .name = "Socket",
    .size = sizeof(Socket),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)io_init_copy,
    .destroy = (destroy_cb)io_destroy,
    .cmp = (cmp_cb)io_cmp,
    .repr = (repr_cb)io_to_cstr,
    .to_cstr = (to_cstr_cb)io_to_cstr,
    // Io Class
    .get_fd = (get_fd_cb)io_get_fd,
    .__read__ = (read_cb)io_read,
    .readstr = (readstr_cb)io_readstr,
    .readline = (readline_cb)io_readline,
    .__write__ = (write_cb)io_write,
    .__close__ = (close_cb)io_close,
};

const io_class *SocketCls = &_SocketCls;
