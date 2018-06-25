#include "JsonParser.h"
#include <assert.h>
#include <stdlib.h>

typedef struct {
    const char *json;
}json_context;

#define EXPECT(c, ch) do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

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

static double json_parse_number(json_context *context, json_value *value) {
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

    value->number = strtod(context->json, &end);
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

double get_json_number(const json_value *value) {
    assert(value != NULL && JSON_NUMBER == value->type);
    return value->number;
}

