#ifndef _MASC_PATH_H_
#define _MASC_PATH_H_

#include <unistd.h>
#include <stdbool.h>

#include <masc/str.h>
#include <masc/array.h>
#include <masc/list.h>
#include <masc/map.h>


typedef enum {
    PATH_FILE,
    PATH_DIR,
    PATH_LINK,
    PATH_OTHER,
    PATH_ERROR
} PathType;


bool path_is_abs(const char *path);
Str *path_abspath(const char *path);

bool path_exists(const char *path);
bool path_is_file(const char *path);
bool path_is_dir(const char *path);
bool path_is_link(const char *path);
PathType path_type(const char *path);

ssize_t path_getsize(const char *path);

Str *path_join(const char *path, const char *other);

Array *path_split(const char *path);
const char *path_basename(const char *path);
Str *path_dirname(const char *path);

Str *path_normpath(const char *path);

List *path_list(const char *path);

Str *path_readlink(const char *path);

#endif /* _MASC_PATH_H_ */
