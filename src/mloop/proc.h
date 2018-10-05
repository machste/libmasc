#ifndef _MASC_MLOOP_PROC_H_
#define _MASC_MLOOP_PROC_H_

#include <masc/object.h>
#include <masc/mloop.h>


struct MlProc {
    Object obj;
    pid_t pid;
    int status;
    bool running;
    ml_proc_cb run_cb;
    ml_proc_done_cb done_cb;
    void *arg;
};


extern const Class *MlProcCls;


int ml_proc_kill(MlProc *self);


#endif /* _MASC_MLOOP_PROC_H_ */
