#ifndef _MASC_MLOOP_EVENT_H_
#define _MASC_MLOOP_EVENT_H_

#include <masc/object.h>


typedef struct MlEvent MlEvent;

typedef void (*ml_event_cb)(MlEvent *self, void *arg);

struct MlEvent {
    Object;
    volatile bool fired;
    ml_event_cb cb;
    void *arg;
};


extern const class *MlEventCls;


MlEvent *ml_event_new(ml_event_cb cb, void *arg);
void ml_event_init(MlEvent *self, ml_event_cb cb, void *arg);

void ml_event_destroy(MlEvent *self);
void ml_event_delete(MlEvent *self);

#define ml_event_fire(self) mloop_event_fire(self)


#endif /* _MASC_MLOOP_EVENT_H_ */