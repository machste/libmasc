/**
 * @file    masc/mloop.h
 *
 * @brief   Main Loop
 *
 * The mloop module is a singleton and can only be initialised once.
 */
#ifndef _MASC_MLOOP_H_
#define _MASC_MLOOP_H_

#include <stdbool.h>


typedef enum {
    ML_FD_READ = 1,
    ML_FD_WRITE = 2,
    ML_FD_BLOCKING = 4,
    ML_FD_EOF = 8,
} ml_fd_flag_t;

typedef unsigned long ml_time_t;

typedef struct MlTimer MlTimer;
typedef struct MlFd MlFd;

typedef void (*ml_timer_cb)(MlTimer *self, void *arg);
typedef void (*ml_fd_cb)(MlFd *self, int fd, ml_fd_flag_t events, void *arg);


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
 * @brief Cancle Timer
 */
bool mloop_fd_cancle(MlFd *self);

/**
 * @brief Delete Timer
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
