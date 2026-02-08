#ifndef _MASC_MLOOP_TIMER_H_
#define _MASC_MLOOP_TIMER_H_

#include <masc/object.h>


typedef unsigned long ml_time_t;

typedef struct MlTimer MlTimer;

typedef void (*ml_timer_cb)(MlTimer *self, void *arg);

struct MlTimer {
    Object;
    int msec;
    ml_time_t time;
    bool pending;
    ml_timer_cb cb;
    void *arg;
};


extern const class *MlTimerCls;


MlTimer *ml_timer_new(ml_timer_cb cb, void *arg);
void ml_timer_init(MlTimer *self, ml_timer_cb cb, void *arg);

void ml_timer_destroy(MlTimer *self);
void ml_timer_delete(MlTimer *self);

#define ml_timer_msec(self) mloop_timer_msec(self)
#define ml_timer_remaining(self) mloop_timer_remaining(self)

#define ml_timer_in(self, msec) mloop_timer_in(self, msec)
#define ml_timer_add(self, msec) mloop_timer_add(self, msec)
#define ml_timer_cancle(self) mloop_timer_cancle(self)

int ml_timer_cmp(MlTimer *self, MlTimer *other);

#endif /* _MASC_MLOOP_TIMER_H_ */
