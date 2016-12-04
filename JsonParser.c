#include "JsonParser.h"
#include <assert.h>
#include <stdlib.h>

typedef struct {
    const char *json;
}json_context;

#define EXPECT(c, ch) do { assert(*c->json == (ch)); c->json++; } while(0)

/* ws = *(%x20 / %x09 / %x0A / %x0D) */
static void json_parse_whitespace(json_context *context) {
    const char *p = context->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
        p ++;
    }
    context->json = p;
}

/* null  = "null" */
static int json_parse_null(json_context *context, json_value *value) {
    EXPECT(context, 'n');

    const char *p = context->json;
    if (p[0] != 'u' || p[1] != 'l' || p[2] != 'l') {
        return JSON_PARSE_INVALID_VALUE;
    }

    context->json += 3;
    value->type = JSON_NULL;
    return JSON_PARSE_OK;
}

static int json_parse_true(json_context *context, json_value *value) {
    EXPECT(context, 't');
    const char *p = context->json;
    if (p[0] != 'r' || p[1] != 'u' || p[2] != 'e') {
        return JSON_PARSE_INVALID_VALUE;
    }

    context->json += 3;
    value->type = JSON_TRUE;
    return JSON_PARSE_OK;
}

static int json_parse_false(json_context *context, json_value *value) {
    EXPECT(context, 'f');
    const char *p = context->json;
    if (p[0] != 'a' || p[1] != 'l' || p[2] != 's' || p[3] != 'e') {
        return JSON_PARSE_INVALID_VALUE;
    }

    context->json += 5;
    value->type = JSON_FALSE;
    return JSON_PARSE_OK;
}

/* value = null / false / true */
static int json_parse_value(json_context *context, json_value *value) {
    switch (*context->json) {
        case 'n': return json_parse_null(context, value);
        case 't': return json_parse_true(context, value);
        case 'f': return json_parse_false(context, value);
        case '\0': return JSON_PARSE_EXPECT_VAVLUE;
        default : return JSON_PARSE_INVALID_VALUE;
    }
}

int json_parse(json_value* value, const char* json) {
    json_context context;
    assert(value != NULL);
    context.json = json;
    value->type = JSON_NULL;
    json_parse_whitespace(&context);
    return json_parse_value(&context, value);
}

json_type get_json_type(const json_value *value) {
    assert(value != NULL);
    return value->type;
}
