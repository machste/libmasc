#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

#include <masc/path.h>
#include <masc/iter.h>
#include <masc/cstr.h>


bool path_is_abs(const char *path)
{
    return (path != NULL && path[0] == '/');
}

Str *path_abspath(const char *path)
{
    if (path_is_abs(path)) {
        return str_new_cstr(path);
    } else {
        char *pwd = getenv("PWD");
        Str *joined = path_join(pwd, path);
        Str *abspath = path_normpath(joined->cstr);
        delete(joined);
        return abspath;
    }
}

bool path_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

bool path_is_file(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0) {
        return S_ISREG(st.st_mode);
    }
    return false;
}

bool path_is_dir(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    return false;
}

bool path_is_link(const char *path)
{
    struct stat lst;
    if (lstat(path, &lst) == 0) {
        return S_ISLNK(lst.st_mode);
    }
    return false;
}

PathType path_type(const char *path)
{
    struct stat lst;
    if (lstat(path, &lst) == 0) {
        if (S_ISREG(lst.st_mode)) {
            return PATH_FILE;
        } else if (S_ISDIR(lst.st_mode)) {
            return PATH_DIR;
        } else if (S_ISLNK(lst.st_mode)) {
            return PATH_LINK;
        } else {
            return PATH_OTHER;
        }
    }
    return PATH_ERROR;
}

ssize_t path_getsize(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

Str *path_join(const char *path, const char *other)
{
    if (path_is_abs(other)) {
        return str_new_cstr(other);
    } else if (cstr_is_empty(path) || cstr_endswith(path, "/") > 0) {
        Str *joined = str_new_cstr(path);
        return str_append(joined, other);
    } else {
        return new(Str, "%s/%s", path, other);
    }
}

Array *path_split(const char *path)
{
    Array *split = new(Array, sizeof(Str), 2);
    const char *split_pos = strrchr(path, '/');
    Str *basename = array_get_at(split, 1);
    if (split_pos != NULL) {
        Str *dirname = array_get_at(split, 0);
        str_init_ncopy(dirname, path, split_pos - path);
        str_init_cstr(basename, split_pos + 1);
    } else {
        str_init_cstr(basename, path);
    }
    return split;
}

const char *path_basename(const char *path)
{
    const char *basename = strrchr(path, '/');
    if (basename != NULL) {
        basename++;
        return basename;
    }
    return path;
}

Str *path_dirname(const char *path)
{
    Array *split = path_split(path);
    Str *dirname = array_remove_at(split, 0);
    delete(split);
    return dirname;
}

Str *path_normpath(const char *path)
{
    Str p_str;
    str_init_cstr(&p_str, path);
    bool initial_slash = str_startswith(&p_str, "/");
    List *parts = str_split(&p_str, "/", -1);
    destroy(&p_str);
    List *new_parts = new(List);
    // Analyse path parts
    Iter *itr = new(Iter, parts);
    for (Str *part = next(itr); part != NULL; part = next(itr)) {
        if (str_is_empty(part) || str_eq_cstr(part, ".")) {
            continue;
        }
        if (!str_eq_cstr(part, "..") ||
                (!initial_slash && list_is_empty(new_parts)) ||
                (!list_is_empty(new_parts) &&
                        str_eq_cstr(list_get_at(new_parts, -1), ".."))) {
            list_append(new_parts, new_copy(part));
        } else {
            list_delete_at(new_parts, -1);
        }
    }
    delete(itr);
    delete(parts);
    // Put parts together
    Str *normpath;
    if (list_is_empty(new_parts) && !initial_slash) {
        normpath = str_new_cstr(".");
    } else {
        normpath = str_join(new_parts, "/");
    }
    delete(new_parts);
    if (initial_slash) {
        str_prepend(normpath, "/");
    }
    return normpath;
}

List *path_list(const char *path)
{
    List *list = new(List);
    DIR* dir = opendir(path);
    if (dir != NULL) {
        struct dirent* direntry;
        while ((direntry = readdir(dir)) != NULL) {
            char *name = direntry->d_name;
            if (cstr_eq(name, ".") || cstr_eq(name, "..")) {
                continue;
            }
            list_append(list, str_new_cstr(name));
        }
    }
    return list;
}

Str *path_readlink(const char *path)
{
    Str *link = NULL;
    struct stat lst;
    if (lstat(path, &lst) == 0 && S_ISLNK(lst.st_mode)) {
        link = str_new_ncopy(NULL, lst.st_size);
        // TODO: Here we have a possible race, when link size changes.
        readlink(path, link->cstr, link->size);
        link->cstr[lst.st_size] = '\0'; // readlink does not terminate cstr!
    }
    return link;
}
