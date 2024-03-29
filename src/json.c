#include <stdio.h>
#include <string.h>

#include <masc/json.h>
#include <masc/map.h>
#include <masc/list.h>
#include <masc/str.h>
#include <masc/int.h>
#include <masc/double.h>
#include <masc/bool.h>
#include <masc/none.h>
#include <masc/cstr.h>
#include <masc/iter.h>
#include <masc/math.h>
#include <masc/regex.h>
#include <masc/macro.h>
#include <masc/print.h>


static char *err2str[] = {
    [JSON_SUCCESS] = "ok",
    [JSON_ERROR_FATAL] = "Fatal parser error",
    [JSON_ERROR_INVAL] = "Invalid character inside JSON string",
    [JSON_ERROR_PART] = "Not a full JSON string, more bytes expected",
    [JSON_ERROR_ROOT] = "Invalid root object",
    [JSON_ERROR_MISSMATCH] = "Missmatch of '}' or ']'",
    [JSON_ERROR_EXTRA_DATA] = "Extra data detected after end of JSON string",
    [JSON_ERROR_COMMA] = "Missing comma detected",
    [JSON_ERROR_COLON] = "Missing colon detected",
    [JSON_ERROR_EXPECT_VALUE] = "Value was expected"
};


static bool is_valid_root(const void *root)
{
    const class *rcls[] = {MapCls, ListCls, StrCls, IntCls, DoubleCls,
        BoolCls, NoneCls
    };
    const class *root_cls = class_of(root);
    for (int i = 0; i < ARRAY_LEN(rcls); i++) {
        if (root_cls == rcls[i]) {
            return true;
        }
    }
    return false;
}

Json *json_new(const void *root)
{
    Json *self = malloc(sizeof(Json));
    json_init(self, root);
    return self;
}

void json_init(Json *self, const void *root)
{
    object_init(self, JsonCls);
    if (is_valid_root(root)) {
        self->root = new_copy(root);
        self->error = JSON_SUCCESS;
    } else {
        self->root = NULL;
        self->error = JSON_ERROR_ROOT;
    }
}

static void _vinit(Json *self, va_list va)
{
    void *root = va_arg(va, void *);
    json_init(self, root);
}

Json *json_new_cstr(const char *cstr)
{
    Json *self = malloc(sizeof(Json));
    json_init_cstr(self, cstr);
    return self;
}

void json_init_cstr(Json *self, const char *cstr)
{
    object_init(self, JsonCls);
    self->error = json_parse_to_obj(&self->root, cstr);
}

Json *json_new_copy(const Json *other)
{
    Json *self = malloc(sizeof(Json));
    json_init_copy(self, other);
    return self;
}

void json_init_copy(Json *self, const Json *other)
{
    object_init(self, JsonCls);
    self->root = new_copy(other->root);
    self->error = other->error;
}

void json_destroy(Json *self)
{
    if (self->root != NULL) {
        delete(self->root);
    }
}

void json_delete(Json *self)
{
    json_destroy(self);
    free(self);
}

bool json_is_valid(Json *self)
{
    return self->error == JSON_SUCCESS;
}

const char *json_err_msg(Json *self)
{
    return err2str[self->error];
}

void *json_get_root(Json *self)
{
    return self->root;
}

bool json_set_root(Json *self, void *root)
{
    if (is_valid_root(root)) {
        if(self->root != NULL) {
            delete(self->root);
        }
        self->root = root;
        self->error = JSON_SUCCESS;
        return true;
    }
    return false;
}

JsonError json_parse(Json *self, const char *cstr)
{
    if(self->root != NULL) {
        delete(self->root);
    }
    return (self->error = json_parse_to_obj(&self->root, cstr));
}

static List *_key_to_tokens(const char *key)
{
    List *tokens = list_new();
    size_t pos = 0;
    size_t tok_start = 0;    
    bool quoted = false;
    bool indexed = false;
    bool error = false;
    Str key_str;
    str_init_cstr(&key_str, key);
    str_strip(&key_str);
    key = key_str.cstr;
    // Parse tokens
    while (key[pos] != '\0') {
        if (key[pos] == '"') {
            if (indexed) {
                quoted = !quoted;
            } else {
                error = true;
                break;
            }
        } else if (key[pos] == '[' && !quoted) {
            if (indexed) {
                error = true;
                break;
            }
            indexed = true;
            if (pos > tok_start) {
                list_append(tokens, str_new_slice(&key_str, tok_start, pos));
            }
            tok_start = pos + 1;
        } else if (key[pos] == ']' && !quoted) {
            if (!indexed) {
                error = true;
                break;
            }
            indexed = false;
        } else if (key[pos] == '.' && !quoted) {
            if (indexed) {
                error = true;
                break;
            }
            if (pos > tok_start) {
                list_append(tokens, str_new_slice(&key_str, tok_start, pos));
            }
            tok_start = pos + 1;
        }
        pos++;
    }
    if (error || quoted || indexed) {
        list_delete(tokens);
        tokens = NULL;
    } else if (pos > tok_start) {
        // Save last token
        list_append(tokens, str_new_slice(&key_str, tok_start, pos));
    }
    str_destroy(&key_str);
    return tokens;    
}

static void *_get_node(void *node, List *tokens)
{
    Iter *itr = new(Iter, tokens);
    for (Str *tok = next(itr); tok != NULL && node != NULL; tok = next(itr)) {
        if (str_is_empty(tok)) {
            node = NULL;
        } else {
            if (str_startswith(tok, "\"") && isinstance(node, Map)) {
                Str stripped_tok;
                str_init_slice(&stripped_tok, tok, 1, -2);
                node =  map_get(node, stripped_tok.cstr);
                str_destroy(&stripped_tok);
            } else if (str_get_at(tok, -1) == ']' && isinstance(node, List)) {
                char *endptr;
                long list_idx = strtol(tok->cstr, &endptr, 10);
                if (*endptr == ']') {
                    node = list_get_at(node, list_idx);
                } else {
                    node = NULL;
                }
            } else if (isinstance(node, Map)) {
                node = map_get(node, tok->cstr);
            } else {
                node = NULL;
            }
        }
    }
    delete(itr);
    return node;
}

void *json_get_node(Json *self, const char *key)
{
    void *node = self->root;
    List *tokens = _key_to_tokens(key);
    if (tokens == NULL) {
        return NULL;
    }
    if (!list_is_empty(tokens)) {
        node = _get_node(self->root, tokens);
    }
    list_delete(tokens);
    return node;
}

bool json_set_node(Json *self, const char *key, void *obj)
{
    bool ret = true;
    List *tokens = _key_to_tokens(key);
    if (tokens == NULL) {
        return false;
    }
    if (list_is_empty(tokens)) {
        // Replace root node
        if (self->root != NULL) {
            delete(self->root);
        }
        self->root = obj;
    } else {
        Str *tok = list_remove_at(tokens, -1); 
        void *prev = _get_node(self->root, tokens);
        if (prev != NULL) {
            if (str_startswith(tok, "\"") && isinstance(prev, Map)) {
                Str stripped_tok;
                str_init_slice(&stripped_tok, tok, 1, -2);
                map_set(prev, stripped_tok.cstr, obj);
                str_destroy(&stripped_tok);
            } else if (str_get_at(tok, -1) == ']' && isinstance(prev, List)) {
                char *endptr;
                long list_idx = strtol(tok->cstr, &endptr, 10);
                if (*endptr == ']') {
                    if (!list_set_at(prev, list_idx, obj)) {
                        size_t len = list_len(prev);
                        for (size_t i = len; i < list_idx; i++) {
                            list_append(prev, NULL);
                        }
                        list_append(prev, obj);
                    }
                }
            } else if (isinstance(prev, Map)) {
                map_set(prev, tok->cstr, obj);
            } else {
                ret = false;
            }
        } else {
            ret = false;
        }
        delete(tok);
    }
    delete(tokens);
    return ret;
}

void *json_remove_node(Json *self, const char *key)
{
    void *node = self->root;
    List *tokens = _key_to_tokens(key);
    if (tokens == NULL) {
        return NULL;
    }
    if (!list_is_empty(tokens)) {
        Str *tok = list_remove_at(tokens, -1); 
        void *prev = _get_node(node, tokens);
        if (prev != NULL) {
            if (str_startswith(tok, "\"") && isinstance(prev, Map)) {
                Str stripped_tok;
                str_init_slice(&stripped_tok, tok, 1, -2);
                node = map_remove_key(prev, stripped_tok.cstr);
                str_destroy(&stripped_tok);
            } else if (str_get_at(tok, -1) == ']' && isinstance(prev, List)) {
                char *endptr;
                long list_idx = strtol(tok->cstr, &endptr, 10);
                if (*endptr == ']') {
                    node = list_remove_at(prev, list_idx);
                } else {
                    node = NULL;
                }
            } else if (isinstance(prev, Map)) {
                node = map_remove_key(prev, tok->cstr);
            } else {
                node = NULL;
            }
        } else {
            node = NULL;
        }
        delete(tok);
    } else {
        // Remove root node
        self->root = NULL;
    }
    delete(tokens);
    return node;
}

bool json_delete_node(Json *self, const char *key)
{
    void *node = json_remove_node(self, key);
    if (node != NULL) {
        delete(node);
        return true;
    }
    return false;
}

size_t json_pretty_cstr(Json *self, char *cstr, size_t size)
{
    return pretty_cstr(self->root, cstr, size);
}

Str *json_pretty_str(Json *self)
{
    size_t len = pretty_cstr(self->root, NULL, 0);
    Str *str = str_new_ncopy(NULL, len);
    pretty_cstr(self->root, str->cstr, len + 1);
    return str;
}

void json_pretty_print(Json *self)
{
    pretty_print(self->root);
}

size_t json_repr(Json *self, char *cstr, size_t size)
{
    return snprintf(cstr, size, "<%s root: %s (%s) at %p>",
            name_of(self), name_of(self->root), err2str[self->error], self);
}

size_t json_to_cstr(Json *self, char *cstr, size_t size)
{
    return to_cstr(self->root, cstr, size);
}

static void _init_class(class *cls)
{
    cls->super = ObjectCls;
}


static class _JsonCls = {
    .name = "Json",
    .size = sizeof(Json),
    .super = NULL,
    .init_class = _init_class,
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)json_init_copy,
    .destroy = (destroy_cb)json_destroy,
    .cmp = (cmp_cb)object_cmp,
    .repr = (repr_cb)json_repr,
    .to_cstr = (to_cstr_cb)json_to_cstr,
};

const class *JsonCls = &_JsonCls;
