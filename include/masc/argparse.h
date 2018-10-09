#ifndef _MASC_ARGPARSE_H_
#define _MASC_ARGPARSE_H_

#include <stdbool.h>

#include <masc/object.h>
#include <masc/list.h>
#include <masc/map.h>
#include <masc/str.h>


typedef enum {
    ARGPARSE_SUCCESS,
    ARGPARSE_ARG_NO_NAME,
    ARGPARSE_INVALID_NARGS,
    ARGPARSE_ERROR
} ArgparseError;

typedef struct {
    Object obj;
    char *prog;
    char *help;
    List opts;
    List args;
} Argparse;

typedef void *(*ap_type_cb)(Str *, Str **);


extern const void *ArgparseCls;


Argparse *argparse_new(const char *prog, const char *help);
void argparse_init(Argparse *self, const char *prog, const char *help);
void argparse_vinit(Argparse *self, va_list va);

void argparse_destroy(Argparse *self);
void argparse_delete(Argparse *self);

ArgparseError argparse_add_arg(Argparse *self, const char *name,
        const char *metavar, const char *nargs, ap_type_cb type_cb,
        const char *help);
ArgparseError argparse_add_opt(Argparse *self, char flag, const char *name,
        const char *metavar, const char *nargs, ap_type_cb type_cb,
        const char *help);

bool argparse_set_default(Argparse *self, const char *name, const char *dfl);

Map *argparse_parse(Argparse *self, int argc, char *argv[]);

void argparse_print_usage(Argparse *self);
void argparse_print_help(Argparse *self);

size_t argparse_to_cstr(Argparse *self, char *cstr, size_t size);

void *argparse_int(Str *integer, Str **err_msg);
void *argparse_num(Str *num, Str **err_msg);
void *argparse_file(Str *path, Str **err_msg);
void *argparse_ip(Str *ip, Str **err_msg);

#endif /* _MASC_ARGPARSE_H_ */
