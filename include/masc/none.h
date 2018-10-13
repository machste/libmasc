#ifndef _MASC_NONE_H_
#define _MASC_NONE_H_

#include <stdbool.h>

#include <masc/object.h>

#define NULLCls NoneCls


typedef struct {
    Object;
} None;


extern const char *const none_cstr;
extern const char *const none_repr;
extern const class *NoneCls;


bool is_none(const Object *obj);


#endif /* _MASC_NONE_H_ */
