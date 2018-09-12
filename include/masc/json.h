#ifndef _MASC_JSON_H_
#define _MASC_JSON_H_

#include <masc/object.h>


typedef enum {
    JSON_SUCCESS,
    JSON_ERROR_FATAL,
    JSON_ERROR_INVAL,
    JSON_ERROR_PART,
    JSON_ERROR_ROOT,
    JSON_ERROR_MISSMATCH,
    JSON_ERROR_EXTRA_DATA,
    JSON_ERROR_COMMA,
    JSON_ERROR_COLON,
    JSON_ERROR_EXPECT_VALUE
} JsonError;

typedef struct {
    Object obj;
    void *root;
    JsonError error;
} Json;


extern const void *JsonCls;


Json *json_new(const void *root);
void json_init(Json *self, const void *root);

Json *json_new_cstr(const char *cstr);
void json_init_cstr(Json *self, const char *cstr);

Json *json_new_copy(const Json *other);
void json_init_copy(Json *self, const Json *other);

void json_destroy(Json *self);
void json_delete(Json *self);

JsonError json_parse(void **root, const char *cstr);

size_t json_to_cstr(Json *self, char *cstr, size_t size);

#endif /* _MASC_JSON_H_ */
