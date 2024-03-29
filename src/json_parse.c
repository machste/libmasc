#include <ctype.h>

#include <masc/json.h>
#include <masc/map.h>
#include <masc/list.h>
#include <masc/str.h>
#include <masc/bool.h>
#include <masc/none.h>
#include <masc/cstr.h>
#include <masc/math.h>


typedef enum {
    EXPECT_VALUE, EXPECT_END, EXPECT_COLON, EXPECT_COMMA
} expect_t;

static JsonError expect2err[] = {
    [EXPECT_VALUE] = JSON_ERROR_EXPECT_VALUE,
    [EXPECT_END] = JSON_ERROR_EXTRA_DATA,
    [EXPECT_COLON] = JSON_ERROR_COLON,
    [EXPECT_COMMA] = JSON_ERROR_COMMA
};


static Str *parse_string(const char *cstr, size_t *pos)
{
    Str *string = NULL;
    bool unrepr = false;
    *pos = 1; // Skip starting quote
    while (cstr[*pos] != '\"') {
        if (cstr[*pos] == '\\') {
            // Skip escape sequence
            (*pos)++;
            // String includes escape sequence "unrepresent" it later
            unrepr = true;
        } else if (cstr[*pos] == '\0') {
            *pos = 0;
            break;
        }
        (*pos)++;
    }
    if (*pos > 0) {
        string = str_new_ncopy(cstr + 1, *pos - 1);
        if (unrepr) {
            str_unrepr(string);
        }
        // Skip ending quote
        (*pos)++;
    }
    return string;
}

static void *parse_number(const char *cstr, size_t *pos)
{
    char *endptr;
    void *num = cstr_to_number(cstr, false, &endptr);
    if (endptr != cstr) {
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

JsonError json_parse_to_obj(void **root, const char *cstr)
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
            size_t offset = 0;
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
