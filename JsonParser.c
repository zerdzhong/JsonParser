#include "JsonParser.h"
#include <assert.h>
#include <memory.h>

typedef struct {
    const char *json;
}json_context;

#define EXPECT(c, ch) do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

#define json_value_init(v) do {(v)->type = JSON_NULL;} while(0);

/* ws = *(%x20 / %x09 / %x0A / %x0D) */
static void json_parse_whitespace(json_context *context) {
    const char *p = context->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
        p ++;
    }
    context->json = p;
}

static int json_parse_literal(json_context *context,
                              json_value *value,
                              const char *literal,
                              json_type type) {

    size_t i = 0;
    EXPECT(context, literal[0]);

    for (i = 0; literal[i + 1]; i++) {
        if (context->json[i] != literal[i + 1]) {
            return JSON_PARSE_INVALID_VALUE;
        }
    }

    context->json += i;
    value->type = type;

    return JSON_PARSE_OK;
}

/*
 * number = [ "-" ] int [ frac ] [ exp ]
 * int = "0" / digit1-9 *digit
 * frac = "." 1*digit
 * exp = ("e" / "E") ["-" / "+"] 1*digit
*/

static int json_parse_number(json_context *context, json_value *value) {
    char* end;

    const char *p = context->json;

    if ('-' == *p) p++;
    if ('0' == *p) {
        p++;
    } else {
        if (!ISDIGIT1TO9(*p)) {
            return JSON_PARSE_INVALID_VALUE;
        }
        for (p++; ISDIGIT(*p); p++);
    }

    if ('.' == *p) {
        p++;
        if (!ISDIGIT(*p)) {
            return JSON_PARSE_INVALID_VALUE;
        }
        for (p++; ISDIGIT(*p); p++);
    }

    if ('e' == *p || 'E' == *p) {
        p++;

        if ('-' == *p || '+' == *p) {
            p++;
        }

        for (p++; ISDIGIT(*p); p++);
    }

    value->u.number = strtod(context->json, &end);
    context->json = end;
    value->type = JSON_NUMBER;

    return JSON_PARSE_OK;
}

/* value = null / false / true */
static int json_parse_value(json_context *context, json_value *value) {
    switch (*context->json) {
        case 'n': return json_parse_literal(context, value, "null", JSON_NULL);
        case 't': return json_parse_literal(context, value, "true", JSON_TRUE);
        case 'f': return json_parse_literal(context, value, "false", JSON_FALSE);
        case '\0': return JSON_PARSE_EXPECT_VALUE;
        default : return json_parse_number(context, value);
    }
}

void value_free(json_value *value) {
    assert(NULL != value);

    if (JSON_STRING == value->type) {
        free(value->u.string.str);
    }

    value->type = JSON_NULL;
}

double json_get_number(const json_value *value) {
    assert(value != NULL && JSON_NUMBER == value->type);
    return value->u.number;
}

void json_set_number(json_value *value, double number)
{
    assert(value != NULL && JSON_NUMBER == value->type);
    value->u.number = number;
}


int json_get_boolean(const json_value *value)
{}

void json_set_boolean(json_value *value, int b)
{}


const char* json_get_string(const json_value *value) {
    assert(value != NULL && JSON_STRING == value->type);
    return value->u.string.str;
}


void json_set_string(json_value *value, const char * s, size_t len) {
    assert(NULL != value && (NULL != s || 0 != len));

    value_free(value);

    value->u.string.str = (char *)malloc(len + 1);
    memcpy(value->u.string.str, s, len);
    value->u.string.str[len] = '\0';
    value->u.string.len = len;

    value->type = JSON_STRING;
}


int json_parse(json_value* value, const char* json) {
    json_context context;
    assert(value != NULL);
    context.json = json;
    value->type = JSON_NULL;
    json_parse_whitespace(&context);
    return json_parse_value(&context, value);
}

json_type json_get_type(const json_value *value) {
    assert(value != NULL);
    return value->type;
}


