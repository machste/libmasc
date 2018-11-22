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
#include <masc/io.h>


typedef enum {
    ML_IO_READ = 1,
    ML_IO_WRITE = 2,
    ML_IO_BLOCKING = 4,
    ML_IO_EOF = 8,
} ml_io_flag_t;

typedef unsigned long ml_time_t;

typedef struct MlTimer MlTimer;
typedef struct MlProc MlProc;
typedef struct MlIo MlIo;
typedef struct MlIoReader MlIoReader;
typedef struct MlIoPkg MlIoPkg;

typedef void (*ml_timer_cb)(MlTimer *self, void *arg);
typedef int (*ml_proc_cb)(void *arg);
typedef void (*ml_proc_done_cb)(MlProc *self, int ret, void *arg);
typedef void (*ml_io_cb)(MlIo *self, int fd, ml_io_flag_t events, void *arg);
typedef void (*ml_io_data_cb)(MlIoReader *self, void *data, size_t size,
                              void *arg);
typedef void (*ml_io_eof_cb)(MlIoReader *self, void *arg);
typedef void (*ml_io_pkg_cb)(MlIoPkg *self, void *data, size_t size, void *arg);

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

struct MlIo {
    Object;
    IoBase *io;
    ml_io_flag_t flags;
    ml_io_cb cb;
    void *arg;
};

struct MlIoReader {
    MlIo;
    ml_io_data_cb data_cb;
    ml_io_eof_cb eof_cb;
};

struct MlIoPkg {
    MlIoReader;
    char sentinel;
    ml_io_pkg_cb pkg_cb;
    void *data;
    size_t size;
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
 * @brief Retrigger an Timer
 *
 * Reuse an existing timer and set a new time in milliseconds, if msec is less
 * than 0 it will use the previous time. This function calculates a new
 * absolute time at the time this function is called.
 *
 * @param msec time in milliseconds
 */
void mloop_timer_in(MlTimer *self, int msec);

/**
 * @brief Add Time to a Timer
 *
 * Reuse an existing timer and add msec to the previous absolute time, if msec
 * is less than 0 it will use the previous set relative time. Use this function
 * for running a timer in continuous mode.
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
 * @brief Add an I/O 
 *
 * Add an I/O to wait for an I/O event.
 * 
 * @param io an I/O object
 * @param flags
 * @param cb callback function
 * @param arg optional argument for the callback function
 *
 * @return the added MlIo object otherwise NULL
 */
MlIo *mloop_io_new(IoBase *io, ml_io_flag_t flags, ml_io_cb cb, void* arg);

/**
 * @brief Add an I/O Reader
 *
 * Add an I/O to wait for incoming data, read all data and call the callback
 * function.
 * 
 * @param io an I/O object
 * @param data_cb callback function for incoming data
 * @param eof_cb callback function when the end of file is reached
 * @param arg optional argument for the callback functions
 *
 * @return the added MlIoReader object otherwise NULL
 */
MlIoReader *mloop_io_reader_new(IoBase *io, ml_io_data_cb data_cb,
        ml_io_eof_cb eof_cb, void* arg);

/**
 * @brief Add an I/O Packager
 *
 * Add an I/O to wait for incoming data, read all data, split it by the given
 * sentinel and call the callback function.
 * 
 * @param io an I/O object
 * @param sen the sentinel which marks the end of each package
 * @param pkg_cb callback function for incoming packages
 * @param eof_cb callback function when the end of file is reached
 * @param arg optional argument for the callback functions
 *
 * @return the added MlIoReader object otherwise NULL
 */
MlIoPkg *mloop_io_pkg_new(IoBase *io, char sen, ml_io_pkg_cb pkg_cb,
        ml_io_eof_cb eof_cb, void* arg);

/**
 * @brief Get Main Loop I/O by I/O object
 */
MlIo *mloop_io_by_io(IoBase *io);

/**
 * @brief Cancle I/O Events
 */
bool mloop_io_cancle(MlIo *self);

/**
 * @brief Delete Main Loop I/O
 */
void mloop_io_delete(MlIo *self);

/**
 * @brief Run the Main Loop
 */
void mloop_run(void);

/**
 * @brief Stop the Main Loop
 */
void mloop_stop(void);


#endif /* _MASC_MLOOP_H_ */
