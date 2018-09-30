#include <unistd.h>
#include <signal.h>

#include <masc/class.h>
#include <masc/mloop.h>
#include <masc/str.h>
#include <masc/print.h>


static void timer1_cb(MlTimer *self, void *arg)
{
    print("timer1: %i ms, %O\n", mloop_run_time(), self);
    mloop_stop();
    mloop_timer_delete(self);
}

static void timer2_cb(MlTimer *self, void *arg)
{
    print("timer2: %i ms, %O\n", mloop_run_time(), self);
    mloop_timer_add(self, -1);
}

static void timer3_cb(MlTimer *self, void *arg)
{
    print("timer3: %i ms, %O\n", mloop_run_time(), self);
    mloop_timer_add(self, mloop_timer_msec(self) + 2);
}

static void timer4_cb(MlTimer *self, void *timer2)
{
    print("timer4: %i ms, %O\n", mloop_run_time(), self);
    mloop_timer_delete(timer2);
    mloop_timer_delete(self);
}

static void sigusr1_cb(int signum)
{
    print("Got signal: %i\n", signum);
}

static void stdin_cb(MlFd *self, int fd, ml_fd_flag_t events, void *arg)
{
    char buf[16];
    Str *line = str_new_cstr("");
    ssize_t bytes;
    while ((bytes = read(fd, buf, sizeof(buf) - 1)) > 0) {
        buf[bytes] = '\0';
        str_append(line, buf);
    }
    print("Line: %O", line);
    if (str_startswith(line, "q")) {
        mloop_stop();
    }
    delete(line);
}

int main(int argc, char *argv[])
{
    signal(SIGUSR1, sigusr1_cb);
    mloop_init();
    mloop_fd_new(STDIN_FILENO, ML_FD_READ, stdin_cb, NULL);
    mloop_timer_new(100, timer1_cb, NULL);
    MlTimer *timer2 = mloop_timer_new(25, timer2_cb, NULL);
    mloop_timer_new(2, timer3_cb, NULL);
    mloop_timer_new(50, timer4_cb, timer2);
    mloop_run();
    mloop_destroy();
    print("End: %i ms\n", mloop_run_time());
    return 0;
}
