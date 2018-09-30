#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/epoll.h>
#include <signal.h>
#include <string.h>

#include <masc/mloop.h>
#include <masc/list.h>
#include <masc/iter.h>
#include <masc/macro.h>
#include "mloop/timer.h"
#include "mloop/fd.h"


static int poll_fd;
static bool initialised = false;
static bool running;
static ml_time_t start_time;
static List timers;
static List mlfds;


static void sigterm_cb(int signum)
{
    mloop_stop();
}

void mloop_init(void)
{
    running = false;
    start_time = 0;
    list_init(&timers);
    list_init(&mlfds);
    poll_fd = epoll_create(32);
    if (poll_fd >= 0) {
        fcntl(poll_fd, F_SETFD, fcntl(poll_fd, F_GETFD) | FD_CLOEXEC);
    }
    signal(SIGINT, sigterm_cb);
    signal(SIGTERM, sigterm_cb);
    initialised = true;
}

void mloop_destroy(void)
{
    initialised = false;
    list_destroy(&timers);
    list_destroy(&mlfds);
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
    if (msec >= 0) { 
        MlTimer *timer = new(MlTimer, cb, arg);
        mloop_timer_add(timer, msec);
        return timer;
    }
    return NULL;
}

void mloop_timer_add(MlTimer *self, int msec)
{
    if (self->pending) {
        mloop_timer_cancle(self);
    }
    // Calculate absolute time to trigger the timer
    self->msec = msec < 0 ? self->msec : msec;
    self->time = mloop_time() + self->msec;
    self->pending = true;
    // Insert the timer to the timer list
    list_sort_in(&timers, self, (cmp_cb)ml_timer_cmp);
}

bool mloop_timer_cancle(MlTimer *self)
{
    if (!self->pending) {
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

MlFd *mloop_fd_new(int fd, ml_fd_flag_t flags, ml_fd_cb cb, void* arg)
{
    if (!(flags & (ML_FD_READ | ML_FD_WRITE))) {
        return NULL;
    }
    MlFd *mlfd = new(MlFd, fd, flags, cb, arg);
    struct epoll_event e;
    memset(&e, 0, sizeof(struct epoll_event));
    // Set event flags
    if (flags & ML_FD_READ) {
        e.events |= EPOLLIN | EPOLLRDHUP;
    }
    if (flags & ML_FD_WRITE) {
        e.events |= EPOLLIN;
    }
    // Set fd flags
    if (!(flags & ML_FD_BLOCKING)) {
        int fl = fcntl(mlfd->fd, F_GETFL, 0);
        fl |= O_NONBLOCK;
        fcntl(mlfd->fd, F_SETFL, fl);
    }
    e.data.ptr = mlfd;
    if (epoll_ctl(poll_fd, EPOLL_CTL_ADD, mlfd->fd, &e) == 0) {
        list_append(&mlfds, mlfd);
    } else {
        delete(mlfd);
        mlfd = NULL;
    }
    return mlfd;
}

MlFd *mloop_fd_by_fd(int fd)
{
    if (fd < 0) {
        return NULL;
    }
    MlFd *mlfd;
    Iter *i = new(Iter, &mlfds);
    for (mlfd = next(i); mlfd != NULL; mlfd = next(i)) {
        if (mlfd->fd == fd) {
            break;
        }
    }
    delete(i);
    return mlfd;
}

bool mloop_fd_cancle(MlFd *self)
{
    list_remove(&mlfds, self);
    if (epoll_ctl(poll_fd, EPOLL_CTL_DEL, self->fd, NULL) != 0) {
        return false;
    }
    return true;
}

void mloop_fd_delete(MlFd *self)
{
    mloop_fd_cancle(self);
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
            }
        } else {
            break;
        }
        timer = list_get_at(&timers, 0);
    }
}

#include <stdio.h>
static void _handle_epoll(int timeout)
{
    static struct epoll_event events[16];
    int nfds = epoll_wait(poll_fd, events, ARRAY_LEN(events), timeout);
    for (int i = 0; i < nfds; i++) {
        MlFd *mlfd = events[i].data.ptr;
        if (mlfd->cb != NULL) {
            ml_fd_flag_t ev = 0;
            if(events[i].events & EPOLLIN) {
                ev |= ML_FD_READ;
            }
            if(events[i].events & EPOLLRDHUP) {
                ev |= ML_FD_EOF;
            }
            if(events[i].events & EPOLLOUT) {
                ev |= ML_FD_WRITE;
            }
            if (ev != 0) { 
                mlfd->cb(mlfd, mlfd->fd, ev, mlfd->arg);
            } else {
                printf("error");
            }
        }
    }
}

void mloop_run(void)
{
    running = true;
    start_time = mloop_time();
    while (running) {
        // Handle expired timers
        _handle_timers();
        // Handle file descriptor events
        _handle_epoll(_next_timer());
    }
}

void mloop_stop(void)
{
    running = false;
}
