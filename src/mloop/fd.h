#ifndef _MASC_MLOOP_FD_H_
#define _MASC_MLOOP_FD_H_

#include <masc/object.h>
#include <masc/mloop.h>


struct MlFd {
    Object obj;
    int fd;
    ml_fd_flag_t flags;
    ml_fd_cb cb;
    void *arg;
};

extern const Class *MlFdCls;


#endif /* _MASC_MLOOP_FD_H_ */
