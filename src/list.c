#include <masc/list.h>
#include <masc/none.h>
#include <masc/math.h>
#include <masc/cstr.h>


struct ListNode {
    ListNode *next;
    void *obj;
};


List *list_new(void)
{
    List *self = malloc(sizeof(List));
    list_init(self);
    return self;
}

void list_init(List *self)
{
    object_init(&self->obj, ListCls);
    self->node = NULL;
}

static void _vinit(List *self, va_list va)
{
    list_init(self);
}

List *list_new_copy(List *other)
{
    List *self = malloc(sizeof(List));
    list_init_copy(self, other);
    return self;
}

void list_init_copy(List *self, List *other)
{
    list_init(self);
    ListNode *node = other->node;
    while (node != NULL) {
        void *new_obj = new_copy(node->obj);
        list_append(self, new_obj);
        node = node->next;
    }
}

static ListNode *listnode_new(void *obj) {
    ListNode *self = malloc(sizeof(ListNode));
    if (obj == NULL) {
        self->obj = new(None);
    } else {
        self->obj = obj;
    }
    self->next = NULL;
    return self;
}

static void listnode_delete(ListNode *self) {
    delete(self->obj);
    free(self);
}

void list_destroy(List *self)
{
    ListNode *node = self->node;
    while (node != NULL) {
        ListNode *old_node = node;
        node = node->next;
        listnode_delete(old_node);
    }
}

void list_delete(List *self)
{
    list_destroy(self);
    free(self);
}

void list_append(List *self, void *obj)
{
    ListNode *new_node = listnode_new(obj);
    ListNode *node = self->node;
    if (node == NULL) {
        self->node = new_node;
    } else {
        while (node->next != NULL) {
            node = node->next;
        }
        node->next = new_node;
    }
}

static void *_next(Iter *itr, List *self)
{
    ListNode *node = itr->ptr;
    if (node != NULL) {
        itr->index++;
        itr->ptr = node->next;
        return node->obj;
    }
    return NULL;
}

static bool _is_last(Iter *itr, List *self)
{
    return itr->ptr == NULL;
}

Iter list_iter(List *self)
{
    Iter i;
    iter_init(&i, self, (next_cb)_next, (is_last_cb)_is_last,
            self->node, -1, NULL);
    return i;
}

void list_for_each(List *self, void (*obj_cb)(void *))
{
    ListNode *node = self->node;
    while (node != NULL) {
        if (node->obj != NULL && class_of(node->obj) != NULL) {
            obj_cb(node->obj);
        } 
        node = node->next;
    }
}

size_t list_to_cstr(List *self, char *cstr, size_t size)
{
    int len = 0;
    len += cstr_ncopy(cstr, "[", size);
    ListNode *node = self->node;
    while (node != NULL) {
        if (node->obj != NULL && class_of(node->obj) != NULL) {
            len += repr(node->obj, cstr + len, max(0, size - len));
        } else {
            len += cstr_ncopy(cstr + len, "null", max(0, size - len));
        }
        if (node->next != NULL) {
            len += cstr_ncopy(cstr + len, ", ", max(0, size - len));
        }
        node = node->next;
    }
    len += cstr_ncopy(cstr + len, "]", max(0, size - len));
    return len;
}


static Class _ListCls = {
    .name = "List",
    .size = sizeof(List),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)list_init_copy,
    .destroy = (destroy_cb)list_destroy,
    .repr = (repr_cb)list_to_cstr,
    .to_cstr = (to_cstr_cb)list_to_cstr,
};

const Class *ListCls = &_ListCls;
