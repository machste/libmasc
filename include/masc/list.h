#ifndef _MASC_LIST_H_
#define _MASC_LIST_H_

#include <stdbool.h>

#include <masc/iter.h>


typedef struct ListNode ListNode;

typedef struct List {
    Iterable;
    ListNode *node;
} List;

typedef void *(*list_obj_cb)(Object *obj, void *arg);


extern const class *ListCls;


List *list_new(void);
void list_init(List *self);

List *list_new_copy(List *other);
void list_init_copy(List *self, List *other);

void list_destroy(List *self);
void list_delete(List *self);

size_t list_len(List *self);
bool list_is_empty(List *self);

bool list_is_in(List *self, Object *obj);

void *list_get_at(List *self, int idx);
bool list_set_at(List *self, int idx, Object *obj);
bool list_insert_at(List *self, int idx, Object *obj);
bool list_insert_after(List *self, int idx, Object *obj);

void list_append(List *self, Object *obj);

bool list_remove(List *self, Object *obj);
void *list_remove_at(List *self, int idx);
bool list_delete_obj(List *self, Object *obj);
bool list_delete_at(List *self, int idx);
void list_delete_all(List *self);

void list_copy(List *self, List *other);

void list_sort(List *self, cmp_cb cb);
void list_sort_in(List *self, Object *obj, cmp_cb cb);

void *list_for_each(List *self, list_obj_cb cb, void *arg);

size_t list_to_cstr(List *self, char *cstr, size_t size);


#endif /* _MASC_LIST_H_ */
