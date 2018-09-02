#ifndef _MASC_NONE_H_
#define _MASC_NONE_H_

#include <stdbool.h>

#include <masc/object.h>

#define NULLCls NoneCls


typedef struct {
    Object obj;
} None;


const char *const none_cstr;
const char *const none_repr;

extern const Class *NoneCls;


bool is_none(void *obj);


#endif /* _MASC_NONE_H_ */
