#ifndef _MASC_LOG_H_
#define _MASC_LOG_H_

#include <stdio.h>
#include <syslog.h>


#ifdef LOG_ENABLED
#define log_debug(x, ...) log_msg(LOG_DEBUG, x, ##__VA_ARGS__)
#define log_info(x, ...) log_msg(LOG_INFO, x, ##__VA_ARGS__)
#define log_notice(x, ...) log_msg(LOG_NOTICE, x, ##__VA_ARGS__)
#define log_warn(x, ...) log_msg(LOG_WARNING, x, ##__VA_ARGS__)
#define log_error(x, ...) log_msg(LOG_ERR, x, ##__VA_ARGS__)
#define log_crit(x, ...) log_msg(LOG_CRIT, x, ##__VA_ARGS__)
#define log_alert(x, ...) log_msg(LOG_ALERT, x, ##__VA_ARGS__)
#define log_emerg(x, ...) log_msg(LOG_EMERG, x, ##__VA_ARGS__)
#else
#define log_debug(x, ...) if(0) {}
#define log_info(x, ...) if(0) {}
#define log_notice(x, ...) if(0) {}
#define log_warn(x, ...) if(0) {}
#define log_error(x, ...) if(0) {}
#define log_crit(x, ...) if(0) {}
#define log_alert(x, ...) if(0) {}
#define log_emerg(x, ...) if(0) {}
#endif /* LOG_ENABLED */

#define log_add_stdout() log_add_stream(stdout)
#define log_add_stderr() log_add_stream(stderr)


extern int log_level;


void log_init(int level);
void log_destroy(void);

void log_msg(int level, const char *msg_fmt, ...);

void log_add_stream(FILE *file);
void log_add_file(const char *path);
void log_add_syslog(const char *ident, int option, int facility);


#endif /* _MASC_LOG_H_ */
