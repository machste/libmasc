#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>

#include <masc/argparse.h>
#include <masc/cstr.h>
#include <masc/int.h>
#include <masc/none.h>
#include <masc/bool.h>
#include <masc/iter.h>
#include <masc/file.h>
#include <masc/print.h>
#include "argparse/arg.h"


Argparse *argparse_new(const char *prog, const char *help)
{
    Argparse *self = malloc(sizeof(Argparse));
    argparse_init(self, prog, help);
    return self;
}

void argparse_init(Argparse *self, const char *prog, const char *help)
{
    object_init(self, ArgparseCls);
    self->prog = strdup(prog);
    self->help = help != NULL ? strdup(help) : NULL;
    list_init(&self->opts);
    list_init(&self->args);
}

void argparse_vinit(Argparse *self, va_list va)
{
    char *prog = va_arg(va, char *);
    char *help = va_arg(va, char *);
    argparse_init(self, prog, help);
}

static void _init_copy(Argparse *self, Argparse *other)
{
    argparse_init(self, other->prog, other->help);
    list_copy(&self->opts, &other->opts);
    list_copy(&self->args, &other->args);
}

void argparse_destroy(Argparse *self)
{
    free(self->prog);
    if (self->help != NULL) {
        free(self->help);
    }
    list_destroy(&self->opts);
    list_destroy(&self->args);
}

void argparse_delete(Argparse *self)
{
    argparse_destroy(self);
    free(self);
}

ArgparseError argparse_add_arg(Argparse *self, const char *name,
        const char *metavar, const char *nargs, ap_type_cb type_cb,
        const char *help)
{
    if (name == NULL || cstr_is_empty(name)) {
        return ARGPARSE_ARG_NO_NAME;
    }
    ApArg *arg = new(ApArg, AP_TYPE_ARGUMENT);
    ArgparseError err;
    if ((err = aparg_parse_nargs(arg, nargs)) == ARGPARSE_SUCCESS) {
        arg->type_cb = type_cb;
        arg->name = strdup(name);
        if (metavar != NULL) {
            arg->metavar = strdup(metavar);
        } else {
            arg->metavar = cstr_upper(strdup(name));
        }
        arg->help = help != NULL ? strdup(help) : NULL;
        list_append(&self->args, arg);
    } else {
        delete(arg);
    }
    return err;
}

ArgparseError argparse_add_opt(Argparse *self, char flag, const char *name,
        const char *metavar, const char *nargs, ap_type_cb type_cb,
        const char *help)
{
    if (flag == '\0' && (name == NULL || cstr_is_empty(name))) {
        return ARGPARSE_ARG_NO_NAME;
    }
    ApArg *arg = new(ApArg, AP_TYPE_OPTION);
    ArgparseError err;
    if ((err = aparg_parse_nargs(arg, nargs)) == ARGPARSE_SUCCESS) {
        arg->type_cb = type_cb;
        arg->name = name != NULL ? strdup(name) : NULL;
        arg->flag.c = flag;
        if (metavar != NULL) {
            arg->metavar = strdup(metavar);
        } else if (name != NULL) {
            arg->metavar = cstr_upper(strdup(name));
        } else if (flag != '\0') {
            arg->metavar = cstr_upper(strndup(&flag, 1));
        }
        arg->help = help != NULL ? strdup(help) : NULL;
        list_append(&self->opts, arg);
    } else {
        delete(arg);
    }
    return err;
}

static ApArg *_arg_by_name(List *args, const char *name)
{
    ApArg *arg = NULL;
    Iter i = init(Iter, args);
    for (ApArg *a = next(&i); a != NULL; a = next(&i)) {
        if (cstr_eq(aparg_dest(a), name)) {
            arg = a;
            break;
        }
    }
    destroy(&i);
    return arg;
}


bool argparse_set_default(Argparse *self, const char *name, const char *dfl)
{
    ApArg *arg = _arg_by_name(&self->opts, name);
    if (arg == NULL) {
        arg = _arg_by_name(&self->args, name);
    }
    if (arg != NULL) {
        aparg_set_default(arg, dfl);
    }
    return arg != NULL;
}

static void _set_default(Map *out_args, ApArg *arg, Str **err)
{
    void *dfl;
    if (arg->type == AP_TYPE_OPTION && arg->n == 0) {
        dfl = new(Bool, false);
    } else {
        if (arg->n == 1 && arg->dfl != NULL) {
            Str *raw_val = str_new_cstr(arg->dfl);
            if (arg->type_cb != NULL) {
                dfl = arg->type_cb(raw_val, err);
                delete(raw_val);
            } else {
                dfl = raw_val;
            }
        } else {
            dfl = new(None);
        }
    }
    map_set(out_args, aparg_dest(arg), dfl);
}

Map *argparse_parse(Argparse *self, int argc, char *argv[])
{
    Map *out_args = new(Map);
    Str *err = NULL;
    List args = init(List);
    for (int i = 1; i < argc; i++) {
        if (cstr_eq(argv[i], "-h") || cstr_eq(argv[i], "--help")) {
            argparse_print_help(self);
            exit(0);
        }
        list_append(&args, str_new_cstr(argv[i]));
    }
    // First parse all options
    Iter *itr = new(Iter, &self->opts);
    for (ApArg *opt = next(itr); opt != NULL && err == NULL; opt = next(itr)) {
        int opt_idx = -1;
        Iter *itr_args = new(Iter, &args);
        for (Str *arg = next(itr_args); arg != NULL; arg = next(itr_args)) {
            if ((opt->name != NULL && str_startswith(arg, "--") &&
                    str_endswith(arg, opt->name)) ||
                    (opt->flag.c != '\0' && str_startswith(arg, "-") &&
                    str_get_at(arg, -1) == opt->flag.c)) {
                opt_idx = iter_get_idx(itr_args);
                break;
            }
        }
        delete(itr_args);
        // Parse option and remove it from args list
        if (opt_idx >= 0) {
            list_delete_at(&args, opt_idx);
            if (opt->n == 0) {
                map_set(out_args, aparg_dest(opt), new(Bool, true));
            } else if (opt->n >= 1) {
                List *opt_vals = new(List);
                for (int i = 0; i < opt->n; i++) {
                    Str *raw_val = list_remove_at(&args, opt_idx);
                    if (raw_val == NULL) {
                        err = str_new("option '%s' expects %i argument(s)!",
                                aparg_dest(opt), opt->n);
                        delete(opt_vals);
                        opt_vals = NULL;
                        break;
                    }
                    void *val = raw_val;
                    if (opt->type_cb != NULL) {
                        val = opt->type_cb(raw_val, &err);
                        delete(raw_val);
                        if (val == NULL || err != NULL) {
                            delete(opt_vals);
                            opt_vals = NULL;
                            break;
                        }
                    }
                    list_append(opt_vals, val);
                }
                if (opt_vals != NULL) {
                    if (list_len(opt_vals) == 1) {
                        void *val = list_remove_at(opt_vals, 0);
                        map_set(out_args, aparg_dest(opt), val);
                        delete(opt_vals);
                    } else {
                        map_set(out_args, aparg_dest(opt), opt_vals);
                    }
                }
            }
        } else {
            _set_default(out_args, opt, &err);
        }
    }
    delete(itr);
    // Check if there are still options in the args list
    if (err == NULL) {
        Iter *itr_args = new(Iter, &args);
        for (Str *arg = next(itr_args); arg != NULL; arg = next(itr_args)) {
            if (str_startswith(arg, "-")) {
                err = str_new("unrecognized argument: %O!", arg);
                break;
            }
        }
        delete(itr_args);
    }
    // Parse positional arguments
    if (err == NULL) {
        Iter *itr = new(Iter, &self->args);
        for (ApArg *pa = next(itr); pa != NULL && err == NULL; pa = next(itr)) {
            // Parse argument and remove it from args list
            if (pa->n < 0) {
                if (list_is_empty(&args)) {
                    if (!pa->required) {
                        _set_default(out_args, pa, &err);
                    } else {
                        err = str_new("too few arguments!");
                    }
                } else {
                    List *vals = new(List);
                    Str *raw_val = list_remove_at(&args, 0);
                    while (raw_val != NULL) {
                        void *val = raw_val;
                        if (pa->type_cb != NULL) {
                            val = pa->type_cb(raw_val, &err);
                            delete(raw_val);
                        }
                        if (val != NULL && err == NULL) {
                            list_append(vals, val);
                        } else {
                            delete(vals);
                            vals = NULL;
                            break;
                        }
                        raw_val = list_remove_at(&args, 0);
                    }
                    if (vals != NULL) {
                        map_set(out_args, aparg_dest(pa), vals);
                    }
                }
            } else if (pa->n == 1 && !pa->required) {
                Str *raw_val = list_remove_at(&args, 0);
                if (raw_val != NULL) {
                    void *val = raw_val;
                    if (pa->type_cb != NULL) {
                        val = pa->type_cb(raw_val, &err);
                        delete(raw_val);
                    }
                    map_set(out_args, aparg_dest(pa), val);
                } else {
                    _set_default(out_args, pa, &err);
                }
            } else {
                List *vals = new(List);
                for (int i = 0; i < pa->n; i++) {
                    Str *raw_val = list_remove_at(&args, 0);
                    if (raw_val != NULL) {
                        void *val = raw_val;
                        if (pa->type_cb != NULL) {
                            val = pa->type_cb(raw_val, &err);
                            delete(raw_val);
                            if (val == NULL || err != NULL) {
                                delete(vals);
                                vals = NULL;
                                break;
                            }
                        }
                        list_append(vals, val);
                    } else {
                        err = str_new("too few arguments!");
                        delete(vals);
                        vals = NULL;
                        break;
                    }
                }
                if (vals != NULL) {
                    if (list_len(vals) == 1) {
                        void *val = list_remove_at(vals, 0);
                        map_set(out_args, aparg_dest(pa), val);
                        delete(vals);
                    } else {
                        map_set(out_args, aparg_dest(pa), vals);
                    }
                }
            }
        }
        delete(itr);
    }
    // If there occured an error, print usage, error and exit immediately.
    if (err != NULL) {
        argparse_print_usage(self);
        fprint(stderr, "%s: error: %O\n", self->prog, err);
        delete(err);
        delete(out_args);
        delete(self);
        exit(-1);
    }
    // Check if there are still arguments left
    if (!list_is_empty(&args)) {
        Str *left_args = str_join(&args, " ");
        err = str_new("too many arguments: %O", left_args);
        delete(left_args);
    }
    destroy(&args);
    return out_args;
}

void argparse_print_usage(Argparse *self)
{
    Str args;
    str_init_cstr(&args, "");
    Iter *itr = new(Iter, &self->opts);
    for (ApArg *arg = next(itr); arg != NULL; arg = next(itr)) {
        Str *arg_usage = aparg_usage(arg);
        if (arg_usage != NULL) {
            str_append_fmt(&args, " %O", arg_usage);
            delete(arg_usage);
        }
    }
    delete(itr);    
    itr = new(Iter, &self->args);
    for (ApArg *arg = next(itr); arg != NULL; arg = next(itr)) {
        Str *arg_usage = aparg_usage(arg);
        if (arg_usage != NULL) {
            str_append_fmt(&args, " %O", arg_usage);
            delete(arg_usage);
        }
    }
    delete(itr);    
    Str usage = init(Str, "usage: %s [-h]%O", self->prog, &args);
    put(&usage);
    destroy(&args);
    destroy(&usage);
}

void argparse_print_help(Argparse *self)
{
    argparse_print_usage(self);
    if (self->help != NULL) {
        print("\n%s\n", self->help);
    }
    Str helps;
    str_init_cstr(&helps, "\noptional arguments:\n"
            " -h, --help         show this help message and exit");
    Iter *itr = new(Iter, &self->opts);
    for (ApArg *arg = next(itr); arg != NULL; arg = next(itr)) {
        Str *arg_help = aparg_help(arg);
        if (arg_help != NULL) {
            str_append_fmt(&helps, "\n%O", arg_help);
            delete(arg_help);
        }
    }
    delete(itr);
    if (!list_is_empty(&self->args)) {
        str_append(&helps, "\n\npositional arguments:");
        itr = new(Iter, &self->args);
        for (ApArg *arg = next(itr); arg != NULL; arg = next(itr)) {
            Str *arg_help = aparg_help(arg);
            if (arg_help != NULL) {
                str_append_fmt(&helps, "\n%O", arg_help);
                delete(arg_help);
            }
        }
        delete(itr);    
    }
    put(&helps);
    destroy(&helps);
}

size_t argparse_to_cstr(Argparse *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "<%s prog: %s at %p>", name_of(self),
            self->prog, self);
}

void *argparse_int(Str *integer, Str **err_msg)
{
    Int *i = new(Int, 0);
    if (!int_set_cstr(i, integer->cstr, true)) {
        *err_msg = str_new("unable to convert '%O' to int!", integer);
        delete(i);
        i = NULL;
    }
    return i;
}

void *argparse_num(Str *num, Str **err_msg)
{
    void *n = str_to_number(num, true);
    if (n == NULL) {
        *err_msg = str_new("unable to convert '%O' to number!", num);
    }
    return n;
}

void *argparse_file(Str *path, Str **err_msg)
{
    File *f = new(File, str_cstr(path), "r");
    if (!file_is_open(f)) {
        *err_msg = str_new("file '%s': %s!", file_path(f), file_err_msg(f));
        delete(f);
        f = NULL;
    }
    return f;
}

void *argparse_ip(Str *ip, Str **err_msg)
{
    struct in_addr addr;
    if (!inet_aton(str_cstr(ip), &addr)) {
        *err_msg = str_new("invalid IP: %O!", ip);
        return NULL;
    }
    return new_copy(ip);
}


static class _ArgparseCls = {
    .name = "Argparse",
    .size = sizeof(Argparse),
    .vinit = (vinit_cb)argparse_vinit,
    .init_copy = (init_copy_cb)_init_copy,
    .destroy = (destroy_cb)argparse_destroy,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)argparse_to_cstr,
    .to_cstr = (to_cstr_cb)argparse_to_cstr,
};

const class *ArgparseCls = &_ArgparseCls;
