/**
 * @file    masc/mloop.h
 *
 * @brief   Main Loop
 *
 * The mloop module is a singleton and can only be initialised once.
 */
#ifndef _MASC_MLOOP_H_
#define _MASC_MLOOP_H_

#include <unistd.h>
#include <stdbool.h>

#include <masc/object.h>


typedef enum {
    ML_FD_READ = 1,
    ML_FD_WRITE = 2,
    ML_FD_BLOCKING = 4,
    ML_FD_EOF = 8,
} ml_fd_flag_t;

typedef unsigned long ml_time_t;

typedef struct MlTimer MlTimer;
typedef struct MlProc MlProc;
typedef struct MlFd MlFd;

typedef void (*ml_timer_cb)(MlTimer *self, void *arg);
typedef int (*ml_proc_cb)(void *arg);
typedef void (*ml_proc_done_cb)(MlProc *self, int ret, void *arg);
typedef void (*ml_fd_cb)(MlFd *self, int fd, ml_fd_flag_t events, void *arg);

struct MlTimer {
    Object;
    int msec;
    ml_time_t time;
    bool pending;
    ml_timer_cb cb;
    void *arg;
};

struct MlProc {
    Object;
    pid_t pid;
    int status;
    bool running;
    ml_proc_cb run_cb;
    ml_proc_done_cb done_cb;
    void *arg;
};

struct MlFd {
    Object;
    int fd;
    ml_fd_flag_t flags;
    ml_fd_cb cb;
    void *arg;
};


/**
 * @brief Initialise the Main Loop
 */
void mloop_init(void);

/**
 * @brief Destroy the Main Loop
 */
void mloop_destroy(void);

/**
 * @brief Check if Main Loop is Initialised
 */
bool mloop_is_init(void);

/**
 * @brief Check if Main Loop is Running
 */
bool mloop_is_running(void);

/**
 * @brief Current Main Loop Time
 *
 * @return internal main loop time in ms
 */
ml_time_t mloop_time(void);

/**
 * @brief Time Since the Main Loop was Started
 */
int mloop_run_time(void);

/**
 * @brief Add a New Timer
 *
 * @param msec time in milliseconds
 * @param cb callback function
 * @param arg optional argument for the callback function
 * 
 * @return the added MlTimer object otherwise NULL
 */
MlTimer *mloop_timer_new(int msec, ml_timer_cb cb, void *arg);

/**
 * @brief Add Existing Timer
 *
 * Reuse an existing timer and set a new time in milliseconds, if msec is -1
 * it will use the previous set time.
 *
 * @param msec time in milliseconds
 */
void mloop_timer_add(MlTimer *self, int msec);

/**
 * @brief Cancle Timer
 */
bool mloop_timer_cancle(MlTimer *self);

/**
 * @brief Delete Timer
 */
void mloop_timer_delete(MlTimer *self);

/**
 * @brief Original Time of the Timer
 */
int mloop_timer_msec(MlTimer *self);

/**
 * @brief Remaining Time of the Timer
 */
int mloop_timer_remaining(MlTimer *self);

/**
 * @brief Add and Run a Process
 *
 * @return the added MlProc object otherwise NULL
 */
MlProc *mloop_proc_new(ml_proc_cb run_cb, ml_proc_done_cb done_cb, void *arg);

/**
 * @brief Check if Process is Running
 */
bool mloop_proc_is_running(MlProc *self);

/**
 * @brief Get PID of the Process
 */
pid_t mloop_proc_pid(MlProc *self);

/**
 * @brief Check if Process Terminated by signal
 *
 * @return the signal number otherwise -1
 */
int mloop_proc_signaled(MlProc *self);

/**
 * @brief Run an Existing Process Again
 */
bool mloop_proc_rerun(MlProc *self);

/**
 * @brief Cancle Process
 */
bool mloop_proc_cancle(MlProc *self);

/**
 * @brief Delete Process
 */
void mloop_proc_delete(MlProc *self);

/**
 * @brief Add a File Descriptor
 *
 * Add a file descriptor to wait for an I/O event.
 * 
 * @param fd the file descriptor
 * @param flags
 * @param cb callback function
 * @param arg optional argument for the callback function
 *
 * @return the added MlFd object otherwise NULL
 */
MlFd *mloop_fd_new(int fd, ml_fd_flag_t flags, ml_fd_cb cb, void* arg);

/**
 * @brief Get MlFd by File Descriptor
 */
MlFd *mloop_fd_by_fd(int fd);

/**
 * @brief Set Blocking Mode
 */
bool mloop_fd_set_blocking(int fd, bool blocking);

/**
 * @brief Cancle File Descriptor Events
 */
bool mloop_fd_cancle(MlFd *self);

/**
 * @brief Delete File Descriptor
 */
void mloop_fd_delete(MlFd *self);

/**
 * @brief Run the Main Loop
 */
void mloop_run(void);

/**
 * @brief Stop the Main Loop
 */
void mloop_stop(void);


#endif /* _MASC_MLOOP_H_ */
