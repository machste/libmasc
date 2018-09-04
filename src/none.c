#include <masc/none.h>
#include <masc/object.h>
#include <masc/cstr.h>

const char *const none_cstr = "None";
const char *const none_repr = "null";


bool is_none(void *obj)
{
    if (obj == NULL || class_of(obj) == NULL || class_of(obj) == NoneCls) {
        return true;
    } else {
        return false;
    }
}

static void _vinit(void *self, va_list _)
{
    object_init(self, NoneCls);
}

static size_t _repr(Object *self, char *cstr, size_t size)
{
    return cstr_ncopy(cstr, none_repr, size);
}

static size_t _to_cstr(Object *self, char *cstr, size_t size)
{
    return cstr_ncopy(cstr, none_cstr, size);
}


static Class _NoneCls = {
    .name = "None",
    .size = sizeof(Object),
    .vinit = _vinit,
    .init_copy = (init_copy_cb)object_init_copy,
    .destroy = (destroy_cb)object_destroy,
    .repr = (repr_cb)_repr,
    .to_cstr = (to_cstr_cb)_to_cstr,
    .iter_init = NULL,
};

const Class *NoneCls = &_NoneCls;
