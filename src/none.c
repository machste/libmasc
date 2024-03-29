#include <masc/none.h>
#include <masc/object.h>
#include <masc/cstr.h>

const char *const none_cstr = "None";
const char *const none_repr = "null";


bool is_none(const Object *obj)
{
    if (obj == NULL || class_of(obj) == NULL || class_of(obj) == NoneCls) {
        return true;
    } else {
        return false;
    }
}

static void _vinit(Object *self, va_list _)
{
    object_init(self, NoneCls);
}

static size_t _repr(const Object *self, char *cstr, size_t size)
{
    return cstr_ncopy(cstr, none_repr, size);
}

static size_t _to_cstr(const Object *self, char *cstr, size_t size)
{
    return cstr_ncopy(cstr, none_cstr, size);
}

static void _init_class(class *cls)
{
    cls->super = ObjectCls;
}


static class _NoneCls = {
    .name = "None",
    .size = sizeof(Object),
    .super = NULL,
    .init_class = _init_class,
    .vinit = _vinit,
    .init_copy = object_init_copy,
    .destroy = object_destroy,
    .cmp = object_cmp,
    .repr = _repr,
    .to_cstr = _to_cstr,
};

const class *NoneCls = &_NoneCls;
