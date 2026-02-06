#include <unistd.h>
#include <signal.h>

#include <masc/class.h>
#include <masc/mloop.h>
#include <masc/str.h>
#include <masc/random.h>
#include <masc/print.h>


MlEvent *event1 = NULL;

static void sigusr1_cb(int signum)
{
    print("sigusr1: signum: %i (%i)\n", signum, getpid());
    mloop_event_fire(event1);
}

static void event1_cb(MlEvent *self, void *arg)
{
    print("event1: %i ms, %O\n", mloop_run_time(), self);
}

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

static void timer3_cb(MlTimer *self, void *proc3)
{
    int run_time = mloop_run_time();
    print("timer3: %i ms, %O\n", run_time, self);
    if (run_time > 70 && mloop_proc_is_running(proc3)) {
        print(" -> cancle proc3: %O\n", proc3);
        mloop_proc_cancle(proc3);
    }
    mloop_timer_in(self, mloop_timer_msec(self) + 2);
}

static void timer4_cb(MlTimer *self, void *timer2)
{
    print("timer4: %i ms, %O\n", mloop_run_time(), self);
    print(" -> delete timer2: %O\n", timer2);
    mloop_timer_delete(timer2);
    mloop_timer_delete(self);
}

static int proc1_run(void *arg)
{
    int i = random_int(10, *(int *)arg);
    usleep(i * 1000);
    return i;
}

static void proc1_done(MlProc *self, int ret, void *arg)
{
    print("proc1: %i ms, %O\n", mloop_run_time(), self);
    mloop_proc_delete(self);
}

static int proc2_run(void *arg)
{
    pid_t ppid = getppid();
    usleep(15000);
    print("proc2: send SIGUSR1 from PID %i to %i\n", getpid(), ppid);
    kill(ppid, SIGUSR1);
    usleep(15000);
    return 42;
}

static void proc2_done(MlProc *self, int ret, void *arg)
{
    print("proc2: %i ms, %O\n", mloop_run_time(), self);
    mloop_proc_rerun(self);
}

static int proc3_run(void *arg)
{
    while (true) {
        sleep(10);
    }
    return 0;
}

static void proc3_done(MlProc *self, int ret, void *arg)
{
    print("proc3: %i ms, %O\n", mloop_run_time(), self);
    // Since timer3 still uses the proc3 reference we do not delete it.
}

int main(int argc, char *argv[])
{
    mloop_init();
    signal(SIGUSR1, sigusr1_cb);
    event1 = mloop_event_new(event1_cb, NULL);
    mloop_timer_new(100, timer1_cb, NULL);
    MlTimer *timer2 = mloop_timer_new(10, timer2_cb, NULL);
    mloop_proc_new(proc1_run, proc1_done, &(int){40});
    mloop_proc_new(proc2_run, proc2_done, event1);
    MlProc *proc3 = mloop_proc_new(proc3_run, proc3_done, NULL);
    mloop_timer_new(2, timer3_cb, proc3);
    mloop_timer_new(50, timer4_cb, timer2);
    mloop_run();
    print("End: %i ms\n", mloop_run_time());
    delete(event1);
    delete(proc3);
    mloop_destroy();
    return 0;
}
