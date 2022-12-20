#ifndef _MASC_SERIAL_H_
#define _MASC_SERIAL_H_

#include <stdio.h>
#include <stdbool.h>

#include <masc/io.h>
#include <masc/str.h>
#include <masc/list.h>


typedef enum {
    SERIAL_SPEED_B0,
    SERIAL_SPEED_B50,
    SERIAL_SPEED_B75,
    SERIAL_SPEED_B110,
    SERIAL_SPEED_B134,
    SERIAL_SPEED_B150,
    SERIAL_SPEED_B200,
    SERIAL_SPEED_B300,
    SERIAL_SPEED_B600,
    SERIAL_SPEED_B1200,
    SERIAL_SPEED_B1800,
    SERIAL_SPEED_B2400,
    SERIAL_SPEED_B4800,
    SERIAL_SPEED_B9600,
    SERIAL_SPEED_B19200,
    SERIAL_SPEED_B38400,
    SERIAL_SPEED_B57600,
    SERIAL_SPEED_B115200,
    SERIAL_SPEED_B230400,
    SERIAL_SPEED_B460800,
    SERIAL_SPEED_B500000,
    SERIAL_SPEED_B576000,
    SERIAL_SPEED_B921600,
    SERIAL_SPEED_B1000000,
    SERIAL_SPEED_MAX
} SerialSpeed;

typedef enum {
    SERIAL_PARITY_NONE,
    SERIAL_PARITY_EVEN,
    SERIAL_PARITY_ODD,
    SERIAL_PARITY_UNKNOWN,
} SerialParity;

typedef struct {
    Io;
    char *path;
    SerialSpeed speed;
    bool parity;
    int errnum;
} Serial;


extern const io_class *SerialCls;


Serial *serial_new(const char *path, SerialSpeed s, SerialParity p);
void serial_init(Serial *self, const char* path, SerialSpeed s, SerialParity p);

void serial_destroy(Serial *self);
void serial_delete(Serial *self);

SerialSpeed serial_baudrate_to_speed(int baudrate);

const char *serial_path(Serial *self);

char *serial_err_msg(Serial *self);

int serial_close(Serial *self);

int serial_cmp(const Serial *self, const Serial *other);

size_t serial_to_cstr(Serial *self, char *cstr, size_t size);

#endif /* _MASC_SERIAL_H_ */
