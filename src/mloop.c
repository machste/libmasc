#include <fcntl.h>
#include <time.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include <masc/mloop.h>
#include <masc/list.h>
#include <masc/iter.h>
#include <masc/macro.h>
#include "mloop/timer.h"
#include "mloop/proc.h"
#include "mloop/io.h"
#include "mloop/ioreader.h"
#include "mloop/iopkg.h"


static int poll_fd;
static bool initialised = false;
static bool running;
static bool got_sigchld = false;
static ml_time_t start_time;
static List timers;
static List procs;
static List mlios;


static void sigterm_cb(int signum)
{
    mloop_stop();
}

static void sigchld_cb(int signum)
{
    got_sigchld = true;
}

void mloop_init(void)
{
    if (initialised) {
        return;
    }
    running = false;
    start_time = 0;
    list_init(&timers);
    list_init(&procs);
    list_init(&mlios);
    poll_fd = epoll_create(32);
    if (poll_fd >= 0) {
        fcntl(poll_fd, F_SETFD, fcntl(poll_fd, F_GETFD) | FD_CLOEXEC);
    }
    signal(SIGINT, sigterm_cb);
    signal(SIGTERM, sigterm_cb);
    signal(SIGCHLD, sigchld_cb);
    initialised = true;
}

void mloop_destroy(void)
{
    initialised = false;
    list_destroy(&timers);
    list_destroy(&procs);
    list_destroy(&mlios);
    close(poll_fd);
}

bool mloop_is_init(void)
{
    return initialised;
}

bool mloop_is_running(void)
{
    return running;
}

ml_time_t mloop_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

int mloop_run_time(void)
{
    if (start_time > 0) {
        return mloop_time() - start_time;
    }
    return -1;
}

MlTimer *mloop_timer_new(int msec, ml_timer_cb cb, void *arg)
{
    if (initialised && msec >= 0) {
        MlTimer *timer = new(MlTimer, cb, arg);
        mloop_timer_in(timer, msec);
        return timer;
    }
    return NULL;
}

static void _timer_set(MlTimer *self, ml_time_t time)
{
    if (self->pending) {
        mloop_timer_cancle(self);
    }
    self->time = time;
    self->pending = true;
    // Insert the timer to the timer list
    list_sort_in(&timers, self, (cmp_cb)ml_timer_cmp);
}

void mloop_timer_in(MlTimer *self, int msec)
{
    if (initialised && msec >= 0) {
        self->msec = msec;
    }
    _timer_set(self, mloop_time() + self->msec);
}

void mloop_timer_add(MlTimer *self, int msec)
{
    if (initialised && msec >= 0) {
        self->msec = msec;
    }
    _timer_set(self, self->time + self->msec);
}

bool mloop_timer_cancle(MlTimer *self)
{
    if (!initialised || !self->pending) {
        return false;
    }
    list_remove(&timers, self);
    self->pending = false;
    return true;
}

void mloop_timer_delete(MlTimer *self)
{
    mloop_timer_cancle(self);
    delete(self);
}

static bool _proc_run(MlProc *self)
{
    pid_t pid = fork();
    if (pid < 0) {
        return false;
    }
    if (pid == 0) {
        int ret = self->run_cb(self->arg);
        exit(ret);
    }
    self->running = true;
    self->pid = pid;
    list_append(&procs, self);
    return true;
}

MlProc *mloop_proc_new(ml_proc_cb run_cb, ml_proc_done_cb done_cb, void *arg)
{
    if (initialised && run_cb == NULL) {
        return NULL;
    }
    MlProc *self = new(MlProc, run_cb, done_cb, arg);
    if (!_proc_run(self)) {
        delete(self);
        self = NULL;
    }
    return self;
}

bool mloop_proc_rerun(MlProc *self)
{
    if (!initialised || self->running) {
        return false;
    }
    return _proc_run(self);
}

bool mloop_proc_cancle(MlProc *self)
{
    if (!initialised || !self->running) {
        return false;
    }
    if (ml_proc_kill(self) != 0) {
        return false;
    }
    return true;
}

void mloop_proc_delete(MlProc *self)
{
    mloop_proc_cancle(self);
    if (initialised) {
        list_remove(&procs, self);
    }
    delete(self);
}

static bool _reg_epoll_events(MlIo *self)
{
    struct epoll_event e;
    memset(&e, 0, sizeof(struct epoll_event));
    // Set event flags
    if (self->flags & ML_IO_READ) {
        e.events |= EPOLLIN | EPOLLRDHUP;
    }
    if (self->flags & ML_IO_WRITE) {
        e.events |= EPOLLOUT;
    }
    // Set fd flags
    if (!(self->flags & ML_IO_BLOCKING)) {
        set_blocking(self->io, false);
    }
    e.data.ptr = self;
    return epoll_ctl(poll_fd, EPOLL_CTL_ADD, get_fd(self->io), &e) == 0;    
}

MlIo *mloop_io_new(IoBase *io, ml_io_flag_t flags, ml_io_cb cb, void* arg)
{
    if (!initialised || !(flags & (ML_IO_READ | ML_IO_WRITE))) {
        return NULL;
    }
    MlIo *self = new(MlIo, io, flags, cb, arg);
    if (_reg_epoll_events(self)) {
        list_append(&mlios, self);
    } else {
        delete(self);
        self = NULL;
    }
    return self;
}

MlIoReader *mloop_io_reader_new(IoBase *io, ml_io_data_cb data_cb,
        ml_io_eof_cb eof_cb, void* arg)
{
    if (!initialised) {
        return NULL;
    }
    MlIoReader *self = new(MlIoReader, io, data_cb, eof_cb, arg);
    if (_reg_epoll_events(self)) {
        list_append(&mlios, self);
    } else {
        delete(self);
        self = NULL;
    }
    return self;
}

MlIoPkg *mloop_io_pkg_new(IoBase *io, char sen, ml_io_pkg_cb pkg_cb,
        ml_io_eof_cb eof_cb, void* arg)
{
    if (!initialised) {
        return NULL;
    }
    MlIoPkg *self = new(MlIoPkg, io, sen, pkg_cb, eof_cb, arg);
    if (_reg_epoll_events(self)) {
        list_append(&mlios, self);
    } else {
        delete(self);
        self = NULL;
    }
    return self;
}

MlIo *mloop_io_by_io(IoBase *io)
{
    if (!initialised || io == NULL) {
        return NULL;
    }
    MlIo *mlio;
    Iter i = init(Iter, &mlios);
    for (mlio = next(&i); mlio != NULL; mlio = next(&i)) {
        if (get_fd(mlio->io) == get_fd(io)) {
            break;
        }
    }
    destroy(&i);
    return mlio;
}

bool mloop_io_cancle(MlIo *self)
{
    if (!initialised) {
        return NULL;
    }
    list_remove(&mlios, self);
    if (epoll_ctl(poll_fd, EPOLL_CTL_DEL, get_fd(self->io), NULL) != 0) {
        return false;
    }
    return true;
}

void mloop_io_delete(MlIo *self)
{
    mloop_io_cancle(self);
    delete(self);
}

static int _next_timer(void)
{
    if (!running) {
        return 0;
    }
    if (list_is_empty(&timers)) {
        return -1;
    }
    int diff = ((MlTimer *)list_get_at(&timers, 0))->time - mloop_time();
    if (diff < 0) {
        return 0;
    }
    return diff;
}

static void _handle_timers(void)
{
    if (list_is_empty(&timers)) {
        return;
    }
    ml_time_t time = mloop_time();
    MlTimer *timer = list_get_at(&timers, 0);
    while (timer != NULL && running) {
        if (timer->time <= time) {
            // Timer expired, remove it from the timer list
            timer = list_remove_at(&timers, 0);
            // Set it as not pending and call the timer callback
            timer->pending = false;
            if (timer->cb != NULL) {
                timer->cb(timer, timer->arg);
            } else {
                delete(timer);
            }
        } else {
            break;
        }
        timer = list_get_at(&timers, 0);
    }
}

static void _handle_processes(void)
{
    pid_t pid;
    int status;
    while (running) {
        pid = waitpid(-1, &status, WNOHANG);
        if (pid < 0 && errno == EINTR) {
            continue;
        }
        if (pid <= 0) {
            return;
        }
        Iter i = init(Iter, &procs);
        for (MlProc *proc = next(&i); proc != NULL; proc = next(&i)) {
            if (proc->pid == pid) {
                // Process finished, remove it from the process list
                list_remove(&procs, proc);
                // Set it as not running and call the done callback
                proc->running = false;
                proc->status = status;
                if (proc->done_cb != NULL) {
                    proc->done_cb(proc, WEXITSTATUS(status), proc->arg);
                } else {
                    delete(proc);
                }
                break;
            }
        }
        destroy(&i);
    }
}

static void _handle_epoll(int timeout)
{
    static struct epoll_event events[16];
    int nfds = epoll_wait(poll_fd, events, ARRAY_LEN(events), timeout);
    for (int i = 0; i < nfds; i++) {
        MlIo *mlio = events[i].data.ptr;
        if (mlio->cb != NULL) {
            ml_io_flag_t ev = 0;
            if(events[i].events & EPOLLIN) {
                ev |= ML_IO_READ;
            }
            if(events[i].events & EPOLLRDHUP) {
                ev |= ML_IO_EOF;
            }
            if(events[i].events & EPOLLOUT) {
                ev |= ML_IO_WRITE;
            }
            if (ev != 0) { 
                mlio->cb(mlio, get_fd(mlio->io), ev, mlio->arg);
            }
        }
    }
}

void mloop_run(void)
{
    // If the main loop is already running, return immediately
    if (running) {
        return;
    }
    // Start the main loop
    running = true;
    start_time = mloop_time();
    while (running) {
        // Handle expired timers
        _handle_timers();
        // Handle terminated child processes
        if (got_sigchld) {
            got_sigchld = false;
            _handle_processes();
        }
        // Handle file descriptor events
        _handle_epoll(_next_timer());
    }
}

void mloop_stop(void)
{
    running = false;
}
