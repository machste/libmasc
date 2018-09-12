#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>

#include <masc/json.h>
#include <masc/map.h>
#include <masc/list.h>
#include <masc/str.h>
#include <masc/num.h>
#include <masc/bool.h>
#include <masc/none.h>
#include <masc/cstr.h>
#include <masc/macro.h>


typedef enum {
    EXPECT_VALUE, EXPECT_END, EXPECT_COLON, EXPECT_COMMA
} expect_t;

static JsonError expect2err[] = {
    [EXPECT_VALUE] = JSON_ERROR_EXPECT_VALUE,
    [EXPECT_END] = JSON_ERROR_EXTRA_DATA,
    [EXPECT_COLON] = JSON_ERROR_COLON,
    [EXPECT_COMMA] = JSON_ERROR_COMMA
};

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
    const Class *rcls[] = {MapCls, ListCls, StrCls, NumCls, BoolCls, NoneCls};
    const Class *root_cls = class_of(root);
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
    object_init(&self->obj, JsonCls);
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
    object_init(&self->obj, JsonCls);
    self->error = json_parse(&self->root, cstr);
}

Json *json_new_copy(const Json *other)
{
    Json *self = malloc(sizeof(Json));
    json_init_copy(self, other);
    return self;
}

void json_init_copy(Json *self, const Json *other)
{
    object_init(&self->obj, JsonCls);
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

static Str *parse_string(const char *cstr, size_t *pos)
{
    Str *string = NULL;
    bool unescape = false;
    *pos = 1; // Skip starting quote
    while (cstr[*pos] != '\"') {
        if (cstr[*pos] == '\\') {
            // Skip escape sequence
            (*pos)++;
            // Unescape string later
            unescape = true;
        } else if (cstr[*pos] == '\0') {
            *pos = 0;
            break;
        }
        (*pos)++;
    }
    if (*pos > 0) {
        string = str_new_ncopy(cstr + 1, *pos - 1);
        if (unescape) {
            str_unescape(string);
        }
        // Skip ending quote
        (*pos)++;
    }
    return string;
}

static Num *parse_number(const char *cstr, size_t *pos)
{
    Num *num = NULL;
    char *endptr;
    double value = strtod(cstr, &endptr);
    if (endptr != cstr) {
        num = new(Num, value);
        *pos = endptr - cstr;
    }
    return num;
}

static Bool *parse_bool(const char *cstr, size_t *pos)
{
    Bool *b = NULL;
    if ((*pos = cstr_startswith(cstr, bool_true_cstr)) > 0) {
        b = new(Bool, true);
    } else if ((*pos = cstr_startswith(cstr, bool_false_cstr)) > 0) {
        b = new(Bool, false);
    }
    return b;
}

static None *parse_null(const char *cstr, size_t *pos)
{
    if ((*pos = cstr_startswith(cstr, none_repr)) > 0) {
        return new(None);
    }
    return NULL;
}

static JsonError append_to_parent(List *stack, void *obj) {
    JsonError err = JSON_SUCCESS;
    void *parent = list_get_at(stack, -1);
    if (parent == NULL) {
        // We got a primitive as first object, put it to the stack.
        list_append(stack, obj);
    } else if (isinstance(parent, List)) {
        list_append(parent, obj);
    } else if (isinstance(parent, Str)) {
        Str *label = list_remove_at(stack, -1);
        parent = list_get_at(stack, -1);
        if (isinstance(parent, Map)) {
            map_set(parent, str_cstr(label), obj);
        } else if (parent == NULL) {
            err = JSON_ERROR_EXTRA_DATA;
        } else {
            err = JSON_ERROR_FATAL;
        }
        delete(label);
    } else {
        err = JSON_ERROR_EXTRA_DATA;
    }
    return err;
}

JsonError json_parse(void **root, const char *cstr)
{
    JsonError err = JSON_SUCCESS;
    size_t pos = 0;
    List *stack = new(List);
    expect_t expect = EXPECT_VALUE;
    // Set root object to NULL
    *root = NULL;
    // Parse JSON string
    while (err == JSON_SUCCESS && cstr[pos] != '\0') {
        if (isspace(cstr[pos])) {
            // Skip spaces
            pos++;
        } else if (cstr[pos] == '{' || cstr[pos] == '[') {
            // Beginn list or map
            if (expect == EXPECT_VALUE) {
                void *obj;
                if (cstr[pos] == '{') {
                    obj = new(Map);
                } else {
                    obj = new(List);
                }
                list_append(stack, obj);
            } else {
                err = expect2err[expect];
            }
            pos++;
        } else if (cstr[pos] == '}' || cstr[pos] == ']') {
            // End of list or map
            size_t stack_len = list_len(stack);
            if (stack_len > 1) {
                void *obj = list_remove_at(stack, -1);
                if ((cstr[pos] == '}' && isinstance(obj, Map)) ||
                        (cstr[pos] == ']' && isinstance(obj, List))) {
                    err = append_to_parent(stack, obj);
                } else {
                    err = JSON_ERROR_MISSMATCH;
                }
                expect = EXPECT_COMMA;
            } else if (stack_len == 1) {
                expect = EXPECT_END;
            } else {
                err = JSON_ERROR_INVAL;
            }
            pos++;
        } else if (cstr[pos] == '\"') {
            // Start string or label
            if (expect == EXPECT_VALUE) {
                size_t offset;
                Str *s = parse_string(cstr + pos, &offset);
                if (s != NULL) {
                    void *parent = list_get_at(stack, -1);
                    if (isinstance(parent, Map)) {
                        // Use string as label
                        list_append(stack, s);
                        expect = EXPECT_COLON;
                    } else {
                        err = append_to_parent(stack, s);
                        expect = EXPECT_COMMA;
                    }
                    pos += offset;
                } else {
                    err = JSON_ERROR_INVAL;
                }
            } else {
                err = expect2err[expect];
            }
        } else if (cstr[pos] == ':') {
            // Label - value seperator
            if (expect == EXPECT_COLON) {
                expect = EXPECT_VALUE;
            } else {
                err = expect2err[expect];
            }
            pos++;
        } else if (cstr[pos] == ',') {
            // Comma
            if (expect == EXPECT_COMMA) {
                expect = EXPECT_VALUE;
            } else {
                err = expect2err[expect];
            }
            pos++;
        } else if (expect == EXPECT_VALUE) {
            void *primitive = NULL;
            size_t offset;
            if (isdigit(cstr[pos]) || cstr[pos] == '-') {
                // Number
                primitive = parse_number(cstr + pos, &offset);
            } else if (cstr[pos] == 't' || cstr[pos] == 'f') {
                // Bool
                primitive = parse_bool(cstr + pos, &offset);
            } else if (cstr[pos] == 'n') {
                // None
                primitive = parse_null(cstr + pos, &offset);
            } else {
                err = JSON_ERROR_INVAL;
            }
            if (primitive != NULL) {
                err = append_to_parent(stack, primitive);
                expect = EXPECT_COMMA;
                pos += offset;
            } else {
                err = JSON_ERROR_INVAL;
            }
        } else {
            err = expect2err[expect];
        }         
    }
    // There should be one object in the stack left, the root object!
    if (err == JSON_SUCCESS) {
        *root = list_remove_at(stack, 0);
    }
    delete(stack);
    return err;
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

static Class _JsonCls = {
    .name = "Json",
    .size = sizeof(Json),
    .vinit = (vinit_cb)_vinit,
    .init_copy = (init_copy_cb)json_init_copy,
    .destroy = (destroy_cb)json_destroy,
    .repr = (repr_cb)json_repr,
    .to_cstr = (to_cstr_cb)json_to_cstr,
    .iter_init = NULL,
};

const void *JsonCls = &_JsonCls;
