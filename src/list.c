#include <masc/list.h>
#include <masc/iter.h>
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

static void listnode_set_obj(ListNode *self, void *obj)
{
    if (obj == NULL) {
        self->obj = new(None);
    } else {
        self->obj = obj;
    }
}

static ListNode *listnode_new(void *obj) {
    ListNode *self = malloc(sizeof(ListNode));
    listnode_set_obj(self, obj);
    self->next = NULL;
    return self;
}

static void listnode_delete(ListNode *self) {
    if (self->obj != NULL) {
        delete(self->obj);
    }
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

size_t list_len(List *self)
{
    size_t len = 0;
    for (ListNode *n = self->node; n != NULL; n = n->next) {
        len++;
    }
    return len;
}

static int _fix_index(List *self, int idx)
{
    size_t len = list_len(self);
    if (len == 0) {
        return -1;
    }
    // Handle negative indexes (e.g. -1 equals to the last index)
    if (idx < 0) {
        idx += len;
        if (idx < 0) {
            // Use first element
            idx = 0;
        }
    } else if (idx >= len) {
        // Use last element
        idx = len;
    }
    return idx;
}

static ListNode *_get_node_at(List *self, int idx)
{
    ListNode *node = NULL;
    if ((idx = _fix_index(self, idx)) >= 0) {
        node = self->node;
        for (int i = 0; i < idx; i++) {
            node = node->next;
        }
    }
    return node;

}

void *list_get_at(List *self, int idx)
{
    void *obj = NULL;
    ListNode *node = _get_node_at(self, idx);
    if (node != NULL) {
        obj = node->obj;
    }
    return obj;
}

void list_set_at(List *self, int idx, void *obj)
{
    ListNode *node = _get_node_at(self, idx);
    if (node != NULL) {
        delete(node->obj);
        listnode_set_obj(node, obj);
    }
}

void list_insert_at(List *self, int idx, void *obj)
{
    int i = _fix_index(self, idx);
    if (i == 0) {
        ListNode *tmp_node = self->node;
        self->node = listnode_new(obj);
        self->node->next = tmp_node;
    } else if (i > 0) {
        ListNode *prev_node = _get_node_at(self, idx - 1);
        if (prev_node != NULL) {
            ListNode *tmp_node = prev_node->next;
            prev_node->next = listnode_new(obj);
            prev_node->next->next = tmp_node;
        }
    }
}

void list_insert_after(List *self, int idx, void *obj)
{
    ListNode *node = _get_node_at(self, idx);
    if (node != NULL) {
        ListNode *next = node->next;
        ListNode *new_node = listnode_new(obj);
        node->next = new_node;
        new_node->next = next;
    }
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

static ListNode *_remove_node_at(List *self, int idx)
{
    ListNode *rm_node = NULL;
    int i = _fix_index(self, idx);
    if (i == 0) {
        rm_node = self->node;
        self->node = rm_node->next;
    } else if (i > 0) {
        ListNode *node = _get_node_at(self, idx - 1);
        if (node != NULL && node->next != NULL) {
            rm_node = node->next;
            node->next = rm_node->next;
        }
    }
    return rm_node;
}

void *list_remove_at(List *self, int idx)
{
    void *obj = NULL;
    ListNode *rm_node = _remove_node_at(self, idx);
    if (rm_node != NULL) {
        obj = rm_node->obj;
        rm_node->obj = NULL;
        listnode_delete(rm_node);
    }
    return obj;    
}

bool list_delete_at(List *self, int idx)
{
    ListNode *rm_node = _remove_node_at(self, idx);
    if (rm_node != NULL) {
        listnode_delete(rm_node);
        return true;
    }
    return false;    
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


typedef struct {
    ListNode *next;
    int idx;
} _IterPriv;

static void *_next(Iter *itr, List *self)
{
    ListNode *node = ((_IterPriv *)itr->priv)->next;
    if (node != NULL) {
        ((_IterPriv *)itr->priv)->idx++;
        ((_IterPriv *)itr->priv)->next = node->next;
        return node->obj;
    }
    return NULL;
}

static bool _is_last(Iter *itr, List *self)
{
    return ((_IterPriv *)itr->priv)->next == NULL;
}

static int _get_idx(Iter *itr, List *self)
{
    return ((_IterPriv *)itr->priv)->idx;
}

static void _iter_init(List *self, Iter *itr)
{
    itr->next = (iter_next_cb)_next;
    itr->is_last = (iter_is_last_cb)_is_last;
    itr->get_idx = (iter_get_idx_cb)_get_idx;
    itr->priv = malloc(sizeof(_IterPriv));
    ((_IterPriv *)itr->priv)->next = self->node;
    ((_IterPriv *)itr->priv)->idx = -1;
    itr->free_priv = free;
}


static Class _ListCls = {
    .name = "List",
    .size = sizeof(List),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)list_init_copy,
    .destroy = (destroy_cb)list_destroy,
    .repr = (repr_cb)list_to_cstr,
    .to_cstr = (to_cstr_cb)list_to_cstr,
    .iter_init = (iter_init_cb)_iter_init,
};

const Class *ListCls = &_ListCls;
