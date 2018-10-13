#ifndef _MASC_MLOOP_TIMER_H_
#define _MASC_MLOOP_TIMER_H_

#include <masc/mloop.h>


extern const class *MlTimerCls;

int ml_timer_cmp(MlTimer *self, MlTimer *other);

#endif /* _MASC_MLOOP_TIMER_H_ */
