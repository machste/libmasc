#ifndef _MASC_MLOOP_TIMER_H_
#define _MASC_MLOOP_TIMER_H_

#include <masc/object.h>
#include <masc/mloop.h>


struct MlTimer {
    Object obj;
    int msec;
    ml_time_t time;
    bool pending;
    ml_timer_cb cb;
    void *arg;
};

extern const Class *MlTimerCls;

int ml_timer_cmp(MlTimer *self, MlTimer *other);

#endif /* _MASC_MLOOP_TIMER_H_ */
