#include <stdlib.h>

#include <masc/list.h>
#include <masc/iter.h>
#include <masc/none.h>
#include <masc/math.h>
#include <masc/utils.h>
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

static void _copy(List *self, List *other)
{
    ListNode *node = other->node;
    while (node != NULL) {
        void *new_obj = new_copy(node->obj);
        list_append(self, new_obj);
        node = node->next;
    }
}

void list_init_copy(List *self, List *other)
{
    list_init(self);
    _copy(self, other);
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

size_t list_len(List *self)
{
    size_t len = 0;
    for (ListNode *n = self->node; n != NULL; n = n->next) {
        len++;
    }
    return len;
}

bool list_is_empty(List *self)
{
    return self->node == NULL;
}

bool list_is_in(List *self, void *obj)
{
    for (ListNode *node = self->node; node != NULL; node = node->next) {
        if (obj == node->obj) {
            return true;
        }
    }
    return false;
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

bool list_set_at(List *self, int idx, void *obj)
{
    ListNode *node = _get_node_at(self, idx);
    if (node != NULL) {
        delete(node->obj);
        listnode_set_obj(node, obj);
        return true;
    }
    return false;
}

bool list_insert_at(List *self, int idx, void *obj)
{
    int i = _fix_index(self, idx);
    if (i == 0 || (self->node == NULL && (idx == 0 || idx == -1))) {
        ListNode *tmp_node = self->node;
        self->node = listnode_new(obj);
        self->node->next = tmp_node;
        return true;
    } else if (i > 0) {
        ListNode *prev_node = _get_node_at(self, idx - 1);
        if (prev_node != NULL) {
            ListNode *tmp_node = prev_node->next;
            prev_node->next = listnode_new(obj);
            prev_node->next->next = tmp_node;
            return true;
        }
    }
    return false;
}

bool list_insert_after(List *self, int idx, void *obj)
{
    ListNode *node = _get_node_at(self, idx);
    if (node != NULL) {
        ListNode *next = node->next;
        ListNode *new_node = listnode_new(obj);
        node->next = new_node;
        new_node->next = next;
        return true;
    }
    return false;
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

static ListNode *_remove_node(List *self, void *obj)
{
    ListNode *node, *prev = NULL;
    for (node = self->node; node != NULL; node = node->next) {
        if (obj == node->obj) {
            if (prev == NULL) {
                self->node = node->next;
            } else {
                prev->next = node->next;
            }
            break;
        }
        prev = node;
    }
    return node;
}

bool list_remove(List *self, void *obj)
{
    ListNode *rm_node = _remove_node(self, obj);
    if (rm_node != NULL) {
        rm_node->obj = NULL;
        listnode_delete(rm_node);
        return true;
    }
    return false;
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

bool list_delete_obj(List *self, void *obj)
{
    ListNode *rm_node = _remove_node(self, obj);
    if (rm_node != NULL) {
        listnode_delete(rm_node);
        return true;
    }
    return false;
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

void list_delete_all(List *self)
{
    list_destroy(self);
    self->node = NULL;
}

void list_copy(List *self, List *other)
{
    list_delete_all(self);
    _copy(self, other);
}


static int _qs_cmp(const void *node_a, const void *node_b, void *arg)
{
    cmp_cb cb = (cmp_cb)arg;
    return cb((*(ListNode **)node_a)->obj, (*(ListNode **)node_b)->obj);
}

void list_sort(List *self, cmp_cb cb)
{
    size_t len = list_len(self);
    if (len == 0) {
        return;
    }
    ListNode **sort_arr = malloc(sizeof(ListNode *) * len);
    size_t i = 0;
    for (ListNode *node = self->node; node != NULL; node = node->next) {
        sort_arr[i++] = node;
    }
    cb = cb == NULL ? cmp : cb;
    quicksort(sort_arr, len, sizeof(ListNode **), _qs_cmp, cb);
    self->node = sort_arr[0];
    for (i = 0; i < len - 1; i++) {
        sort_arr[i]->next = sort_arr[i + 1];
    }
    sort_arr[i]->next = NULL;
    free(sort_arr);
}

void list_sort_in(List *self, void *obj, cmp_cb cb)
{
    cb = cb == NULL ? cmp : cb;
    // Search right position to insert the new object
    ListNode *prev = NULL;
    for (ListNode *node = self->node; node != NULL; node = node->next) {
        if (cb(node->obj, obj) > 0) {
            break;
        }
        prev = node;
    }
    // Insert before current node
    ListNode *new_node = listnode_new(obj);
    ListNode *tmp_node;
    if (prev != NULL) {
        tmp_node = prev->next;
        prev->next = new_node;
    } else {
        tmp_node = self->node;
        self->node = new_node;
    }
    new_node->next = tmp_node;
}

void *list_for_each(List *self, list_obj_cb cb, void *arg)
{
    void *ret = NULL;
    for (ListNode *n = self->node; n != NULL && ret == NULL; n = n->next) {
        ret = cb(n->obj, arg);
    }
    return ret;
}

size_t list_to_cstr(List *self, char *cstr, size_t size)
{
    long len = 0;
    len += cstr_ncopy(cstr, "[", size);
    for (ListNode *node = self->node; node != NULL; node = node->next) {
        if (node->obj != NULL && class_of(node->obj) != NULL) {
            len += repr(node->obj, cstr + len, max(0, size - len));
        } else {
            len += cstr_ncopy(cstr + len, "null", max(0, size - len));
        }
        if (node->next != NULL) {
            len += cstr_ncopy(cstr + len, ", ", max(0, size - len));
        }
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

static void _del_obj(Iter *itr, List *self)
{
    list_delete_at(self, ((_IterPriv *)itr->priv)->idx);
    ((_IterPriv *)itr->priv)->idx--;
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
    itr->del_obj = (iter_del_obj_cb)_del_obj;
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
    .len = (len_cb)list_len,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)list_to_cstr,
    .to_cstr = (to_cstr_cb)list_to_cstr,
    .iter_init = (iter_init_cb)_iter_init,
};

const Class *ListCls = &_ListCls;
