#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <fcntl.h> 
#include <string.h>
#include <errno.h>

#include <masc/serial.h>
#include <masc/path.h>
#include <masc/iter.h>
#include <masc/print.h>


typedef struct {
    unsigned int baudrate;
    speed_t code;
} SpeedEntry;

typedef struct {
    const char *name;
    tcflag_t flags;
} ParityEntry;


static SpeedEntry speed_entries[] = {
    [SERIAL_SPEED_B0] = {0, B0},
    [SERIAL_SPEED_B50] = {50, B50},
    [SERIAL_SPEED_B75] = {75, B75},
    [SERIAL_SPEED_B110] = {110, B110},
    [SERIAL_SPEED_B134] = {134, B134},
    [SERIAL_SPEED_B150] = {150, B150},
    [SERIAL_SPEED_B200] = {200, B200},
    [SERIAL_SPEED_B300] = {300, B300},
    [SERIAL_SPEED_B600] = {600, B600},
    [SERIAL_SPEED_B1200] = {1200, B1200},
    [SERIAL_SPEED_B1800] = {1800, B1800},
    [SERIAL_SPEED_B2400] = {2400, B2400},
    [SERIAL_SPEED_B4800] = {4800, B4800},
    [SERIAL_SPEED_B9600] = {9600, B9600},
    [SERIAL_SPEED_B19200] = {19200, B19200},
    [SERIAL_SPEED_B38400] = {38400, B38400},
    [SERIAL_SPEED_B57600] = {57600, B57600},
    [SERIAL_SPEED_B115200] = {115200, B115200},
    [SERIAL_SPEED_B230400] = {230400, B230400},
    [SERIAL_SPEED_B460800] = {460800, B460800},
    [SERIAL_SPEED_B500000] = {500000, B500000},
    [SERIAL_SPEED_B576000] = {576000, B576000},
    [SERIAL_SPEED_B921600] = {921600, B921600},
    [SERIAL_SPEED_B1000000] = {1000000, B1000000}
};

static ParityEntry parity_entries[] = {
    [SERIAL_PARITY_NONE] = {"None", 0},
    [SERIAL_PARITY_EVEN] = {"Even", PARENB},
    [SERIAL_PARITY_ODD] = {"Odd", (PARENB & PARODD)}
};


Serial *serial_new(const char *path, SerialSpeed s, SerialParity p)
{
    Serial *self = malloc(sizeof(Serial));
    serial_init(self, path, s, p);
    return self;
}

static bool _set_attrs(Serial *self)
{
    // Read current attributes from the serial device
    struct termios tty;
    if (tcgetattr(self->fd, &tty) != 0) {
        return false;
    }
    // Set baudrate (speed)
    speed_t speed_code = speed_entries[self->speed].code;
    cfsetospeed(&tty, speed_code);
    cfsetispeed(&tty, speed_code);
    // Ignore BREAK condition on input
    tty.c_iflag |= IGNBRK;
    // Disable Xon/Xoff flow control
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    // Disable all output options (e.g. remapping, delays, ...)
    tty.c_oflag = 0;
    // Disable all local mode options (e.g. echo, signals, canon mode, ...)
    tty.c_lflag = 0;
    // Set number of bits per symbol (8-bit)
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    // Enable receiver and ignore model control lines
    tty.c_cflag |= (CREAD | CLOCAL);
    // Set parity
    tty.c_cflag |= parity_entries[self->parity].flags;
    // Set serial interface timing
    tty.c_cc[VMIN]  = 1;            // block until 1 char is received
    tty.c_cc[VTIME] = 1;            // 0.1 seconds intercharacter timeout
    // Set all attributes to the serial device
    if (tcsetattr(self->fd, TCSANOW, &tty) != 0) {
        return false;
    }
    return true;
}

void serial_init(Serial *self, const char *path, SerialSpeed s, SerialParity p)
{
    object_init(self, SerialCls);
    self->fd = -1;
    if (s >= SERIAL_SPEED_MAX || p >= SERIAL_PARITY_UNKNOWN) {
        self->errnum = EINVAL;
        return;
    }
    self->speed = s;
    self->parity = p;
    self->path = strdup(path);
    if (path_is_dir(path)) {
        self->errnum = EISDIR;
        return;
    }
    // Open serial device
    self->fd = open(self->path, O_RDWR | O_NOCTTY | O_SYNC);
    if (self->fd < 0) {
        self->errnum = errno;
        return;
    }
    // Check if it is a TTY and set terminal settings
    if (!isatty(self->fd) || !_set_attrs(self)) {
        self->errnum = errno;
        close(self);
        self->fd = -1;
    }
}

static void _vinit(Serial *self, va_list va)
{
    char *path = va_arg(va, char *);
    SerialSpeed speed = va_arg(va, SerialSpeed);
    SerialParity parity = va_arg(va, SerialParity);
    serial_init(self, path, speed, parity);
}

void serial_destroy(Serial *self)
{
    free(self->path);
    close(self);
}

void serial_delete(Serial *self)
{
    serial_destroy(self);
    free(self);
}

SerialSpeed serial_baudrate_to_speed(int baudrate)
{
    SerialSpeed speed = SERIAL_SPEED_B0;
    while (speed_entries[speed].baudrate < baudrate) {
        speed++;
    }
    return speed;
}

const char *serial_path(Serial *self)
{
    return self->path;
}

char *serial_err_msg(Serial *self)
{
    return strerror(self->errnum);
}

int serial_close(Serial *self)
{
    self->errnum = 0;
    return io_close(self);
}

int serial_cmp(const Serial *self, const Serial *other)
{
    return strcmp(self->path, other->path);
}

size_t serial_to_cstr(Serial *self, char *cstr, size_t size)
{
    char *state;
    if (is_open(self)) {
        state = "open";
    } else if (self->errnum != 0) {
        state = strerror(self->errnum);
    } else {
        state = "closed";
    } 
    return snprintf(cstr, size, "<%s '%s', %i 8%c1, %s at %p>",
            name_of(self), self->path, speed_entries[self->speed].baudrate,
            parity_entries[self->parity].name[0], state, self);
}

static void _init_class(class *cls)
{
    cls->super = IoCls;
}


static io_class _SerialCls = {
    .name = "Serial",
    .size = sizeof(Serial),
    .super = NULL,
    .init_class = _init_class,
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)serial_destroy,
    .cmp = (cmp_cb)serial_cmp,
    .repr = (repr_cb)serial_to_cstr,
    .to_cstr = (to_cstr_cb)serial_to_cstr,
    // Io Class
    .get_fd = (get_fd_cb)io_get_fd,
    .__read__ = (read_cb)io_read,
    .readstr = (readstr_cb)io_readstr,
    .readline = (readline_cb)io_readline,
    .__write__ = (write_cb)io_write,
    .__close__ = (close_cb)serial_close,
};

const io_class *SerialCls = &_SerialCls;
