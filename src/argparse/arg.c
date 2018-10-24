#include <stdlib.h>
#include <string.h>

#include "arg.h"
#include <masc/cstr.h>


#define AP_HELP_TAB_POS 20


static void _vinit(ApArg *self, va_list va)
{
    object_init(self, ApArgCls);
    self->type = va_arg(va, ApType);
    self->name = NULL;
    self->flag.c = '\0';
    self->flag.cstr[sizeof(self->flag.cstr) - 1] = '\0';
    self->metavar = NULL;
    self->required = false;
    self->n = 1;
    self->dfl = NULL;
    self->type_cb = NULL;
    self->help = NULL;
}

static void _init_copy(ApArg *self, ApArg *other)
{
    self->type = other->type;
    self->name = other->name != NULL ? strdup(other->name) : NULL;
    self->flag.c = other->flag.c;
    self->metavar = other->metavar != NULL ? strdup(other->metavar) : NULL;
    self->required = other->required;
    self->n = other->n;
    self->dfl = other->dfl != NULL ? strdup(other->dfl) : NULL;
    self->type_cb = other->type_cb;
    self->help = other->help != NULL ? strdup(other->help) : NULL;
}

static void _destroy(ApArg *self)
{
    if (self->name != NULL) {
        free(self->name);
    }
    if (self->metavar != NULL) {
        free(self->metavar);
    }
    if (self->dfl != NULL) {
        free(self->dfl);
    }
    if (self->help != NULL) {
        free(self->help);
    }
}

ArgparseError aparg_parse_nargs(ApArg *self, const char *nargs)
{
    if (nargs == NULL) {
        if (self->type == AP_TYPE_OPTION) {
            self->n = 0;
        } else {
            self->required = true;
            self->n = 1;
        }
    } else if (cstr_eq(nargs, "*")) {
        self->n = -1;
    } else if (cstr_eq(nargs, "?")) {
        self->n = 1;
    } else if (cstr_eq(nargs, "+")) {
        self->required = true;
        self->n = -1;
    } else {
        char *endptr;
        self->required = true;
        self->n = strtol(nargs, &endptr, 10);
        if (*endptr != '\0') {
            return ARGPARSE_INVALID_NARGS;
        }
    }
    return ARGPARSE_SUCCESS;
}

const char *aparg_dest(ApArg *self)
{
    if (self->name != NULL) {
        return self->name;
    } else {
        return self->flag.cstr;
    }
}

void aparg_set_default(ApArg *self, const char *dfl)
{
    if (self->dfl != NULL) {
        free(self->dfl);
    }
    self->dfl = dfl != NULL ? strdup(dfl) : NULL;
}

Str *aparg_usage(ApArg *self)
{
    Str *usage = NULL;
    if (self->type == AP_TYPE_OPTION) {
        usage = str_new_cstr("[");
        if (self->flag.c != '\0') {
            str_append_fmt(usage,"-%c", self->flag.c);
        } else if (self->name != NULL) {
            str_append_fmt(usage,"--%s", self->name);
        }
        if (self->required && self->n > 0) {
            for (int i = 0; i < self->n; i++) {
                str_append_fmt(usage, " %s", self->metavar);
            }
        } else if (!self->required && self->n == 1) {
            str_append_fmt(usage, " [%s]", self->metavar);
        }
        str_append(usage, "]");
    } else if (self->type == AP_TYPE_ARGUMENT) {
        if (self->required) {
            usage = str_new_cstr(self->metavar);
            if (self->n > 1) {
                for (int i = 1; i < self->n; i++) {
                    str_append_fmt(usage, " %s", self->metavar);
                }
            } else if (self->n < 0) {
                str_append_fmt(usage, " [%s ...]", self->metavar);
            }
        } else {
            if (self->n > 0) {
                usage = str_new("[%s]", self->metavar);
            } else {
                usage = str_new("[%s [%s ...]]", self->metavar, self->metavar);
            }
        }
    }
    return usage;
}

Str *aparg_help(ApArg *self)
{
    Str *help = NULL;
    if (self->type == AP_TYPE_OPTION) {
        Str *metavar = str_new_cstr("");
        if (self->required && self->n > 0) {
            for (int i = 0; i < self->n; i++) {
                str_append_fmt(metavar, " %s", self->metavar);
            }
        } else if (!self->required && self->n == 1) {
            str_append_fmt(metavar, " [%s]", self->metavar);
        }
        help = str_new_cstr(" ");
        if (self->flag.c != '\0') {
            str_append_fmt(help,"-%c%O", self->flag.c, metavar);
        }
        if (self->name != NULL) {
            if (self->flag.c != '\0') {
                str_append(help,", ");
            }
            str_append_fmt(help,"--%s%O", self->name, metavar);
        }
        delete(metavar);
    } else if (self->type == AP_TYPE_ARGUMENT) {
        help = str_new(" %s", self->metavar);
    }
    if (self->help != NULL) {
        char *space;
        int spaces = AP_HELP_TAB_POS - str_len(help);
        if (spaces > 0) {
            space = malloc(spaces + 1);
            memset(space, ' ', spaces);
            space[spaces] = '\0';
        } else {
            space = malloc(AP_HELP_TAB_POS + 2);
            space[0] = '\n';
            memset(space + 1, ' ', AP_HELP_TAB_POS);
            space[AP_HELP_TAB_POS + 1] = '\0';
        }
        str_append_fmt(help, "%s%s", space, self->help);
        free(space);
    }
    return help;
}


static class _ApArgCls = {
    .name = "ApArg",
    .size = sizeof(ApArg),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)_init_copy,
    .destroy = (destroy_cb)_destroy,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)object_to_cstr,
    .to_cstr = (to_cstr_cb)object_to_cstr,
};

const class *ApArgCls = &_ApArgCls;
