#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <masc/log.h>
#include <masc/object.h>
#include <masc/str.h>
#include <masc/list.h>


typedef struct LogFacility LogFacility;

typedef enum {
    LOG_FACILITY_STREAM,
    LOG_FACILITY_FILE,
    LOG_FACILITY_SYSLOG,
    LOG_FACILITY_CUSTOM,
    LOG_FACILITY_NONE
} log_facility_t;

typedef void (*log_write_cb)(LogFacility *self, int level, Str *msg);

struct LogFacility {
    Object;
    log_facility_t type;
    log_write_cb write_cb; 
    union {
        FILE *file;
        int fd;
        int syslog_facility;
        void *custom;
    } sink;
};


static const class *LogFacilityCls;

static const char *level_to_cstr[] = {
    [LOG_EMERG] = "emerg",
    [LOG_ALERT] = "alert",
    [LOG_CRIT] = "crit",
    [LOG_ERR] = "error",
    [LOG_WARNING] = "warn",
    [LOG_NOTICE] = "notice",
    [LOG_INFO] = "info",
    [LOG_DEBUG] = "debug"
};

int log_level = LOG_EMERG;
List facilities;


void log_init(int level)
{
    log_level = level;
    list_init(&facilities);
}

void log_destroy(void)
{
    log_level = LOG_EMERG;
    list_destroy(&facilities);
}

struct log_msg {
    int level;
    Str msg;
};

static void *facility_cb(LogFacility *facilitiy, struct log_msg *lmsg)
{
    facilitiy->write_cb(facilitiy, lmsg->level, &lmsg->msg);
    return NULL;
}


void log_msg(int level, const char *msg_fmt, ...)
{
    if(log_level >= level && !list_is_empty(&facilities)) {
        va_list va;
        va_start(va, msg_fmt);
        struct log_msg lmsg;
        lmsg.level = level;
        str_vinit(&lmsg.msg, msg_fmt, va);
        list_for_each(&facilities, (list_obj_cb)facility_cb, &lmsg);
        str_destroy(&lmsg.msg);
        va_end(va);
    }
}

static void _file_write_cb(LogFacility *self, int level, Str *msg)
{
    str_prepend_fmt(msg, "%s: ", level_to_cstr[level]);
    str_append(msg, "\n");
    fwrite(msg->cstr, 1, msg->size - 1, self->sink.file);
    fflush(self->sink.file);
}

void log_add_stream(FILE *file)
{
    LogFacility *f = new(LogFacility);
    f->type = LOG_FACILITY_STREAM;
    f->sink.file = file;
    f->write_cb = _file_write_cb;
    list_append(&facilities, f);
}

void log_add_file(const char *path)
{
    LogFacility *f = new(LogFacility);
    f->sink.file = fopen(path, "w");
    if (f->sink.file != NULL) {
        f->type = LOG_FACILITY_FILE;
        f->write_cb = _file_write_cb;
        list_append(&facilities, f);
    } else {
        log_error("could not open '%s'!", path);
    }
}

static void _syslog_write_cb(LogFacility *self, int level, Str *msg)
{
    syslog(level, "%s", msg->cstr);
}

void log_add_syslog(const char *ident, int option, int facility)
{
    LogFacility *f = new(LogFacility);
    f->type = LOG_FACILITY_SYSLOG;
    openlog(ident, option, facility);
    f->write_cb = _syslog_write_cb;
    list_append(&facilities, f);
}

static void _vinit(LogFacility *self, va_list va)
{
    object_init(self, LogFacilityCls);
    self->type = LOG_FACILITY_NONE;
}

static void _destroy(LogFacility *self)
{
    switch(self->type) {
    case LOG_FACILITY_FILE:
        fclose(self->sink.file);
        break;
    case LOG_FACILITY_SYSLOG:
        closelog();
        break;
    default:
        break;
    }
}


static class _LogFacilityCls = {
    .name = "LogFacility",
    .size = sizeof(LogFacility),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)_destroy,
    .len = (len_cb)object_len,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)object_to_cstr,
    .to_cstr = (to_cstr_cb)object_to_cstr,
    .iter_init = NULL,
};

static const class *LogFacilityCls = &_LogFacilityCls;
