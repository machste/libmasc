#ifndef _INTERNAL_MLOOP_TIMER_H_
#define _INTERNAL_MLOOP_TIMER_H_

#include <masc/mloop.h>


extern const class *MlTimerCls;

int ml_timer_cmp(MlTimer *self, MlTimer *other);

#endif /* _INTERNAL_MLOOP_TIMER_H_ */
