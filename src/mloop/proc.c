#include <signal.h>
#include <stdio.h>

#include "proc.h"


static void _vinit(MlProc *self, va_list va)
{
    object_init(self, MlProcCls);
    self->run_cb = va_arg(va, ml_proc_cb);
    self->done_cb = va_arg(va, ml_proc_done_cb);
    self->arg = va_arg(va, void *);
    self->pid = -1;
    self->status = -1;
    self->running = false;
}

bool mloop_proc_is_running(MlProc *self)
{
    return self->running;
}

pid_t mloop_proc_pid(MlProc *self)
{
    return self->pid;
}

int mloop_proc_signaled(MlProc *self)
{
    if (WIFSIGNALED(self->status)) {
        return WTERMSIG(self->status);
    }
    return -1;
}

int ml_proc_kill(MlProc *self)
{
    int ret = -1;
    if (self->running) {
        ret = kill(self->pid, SIGKILL);
        if (ret == 0) {
            self->running = false;
        }
    }
    return ret;
}

static void _destroy(MlProc *self)
{
    ml_proc_kill(self);
}

static size_t _to_cstr(MlProc *self, char *cstr, size_t size)
{
    if (self->running) {
        return snprintf(cstr, size, "<%s pid: %i, running at %p>",
                name_of(self), self->pid, self);
    }
    int signum = mloop_proc_signaled(self);
    if (signum > 0) {
        return snprintf(cstr, size, "<%s pid: %i, terminated (sig: %i) at %p>",
                name_of(self), self->pid, signum, self);
    } else {
        return snprintf(cstr, size, "<%s pid: %i, exited with %i at %p>",
                name_of(self), self->pid, WEXITSTATUS(self->status), self);
    }
}


static class _MlProcCls = {
    .name = "MlProc",
    .size = sizeof(MlProc),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)_destroy,
    .len = (len_cb)object_len,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)_to_cstr,
    .to_cstr = (to_cstr_cb)_to_cstr,
};

const class *MlProcCls = &_MlProcCls;
