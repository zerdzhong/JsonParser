#include "JsonParser.h"
#include <assert.h>
#include <memory.h>

typedef struct {
    const char *json;
    char * stack;
    size_t size, top;
}json_context;

#define EXPECT(c, ch) do { assert(*(c)->json == (ch)); (c)->json++; } while(0)
#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch) >= '1' && (ch) <= '9')

#ifndef JSON_PARSE_STACK_INIT_SIZE
#define JSON_PARSE_STACK_INIT_SIZE 256
#endif

static void* json_context_push(json_context *context, size_t size) {
    void * ret;
    assert(size > 0);

    if (context->top + size > context->size) {
        if (0 == context->size) {
            context->size = JSON_PARSE_STACK_INIT_SIZE;
        }

        while (context->top + size >= context->size) {
            /* 1.5 times */
            context->size += context->size >> 1;
        }

        context->stack = (char *)realloc(context->stack, context->size);
    }

    ret = context->stack + context->top;
    context->top += size;
    return ret;
}

static void* json_context_pop(json_context *context, size_t size) {
    assert(context->top >= size);
    return context->stack + (context->top -= size);
}

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


#define PUTC(c, ch) do { *(char*)json_context_push(c, sizeof(char)) = (ch); } while(0)
#define STRING_PARSE_ERR(e) do {context->top = head;return e;} while(0)



static const char* json_parse_hex(const char *p, unsigned *unicode){
    int i = 0;
    *unicode = 0;
    for (i = 0; i < 4; ++i) {
        char ch = *p++;
        *unicode <<= 4;
        if      (ch >= '0' && ch <= '9')  *unicode |= ch - '0';
        else if (ch >= 'A' && ch <= 'F')  *unicode |= ch - ('A' - 10);
        else if (ch >= 'a' && ch <= 'f')  *unicode |= ch - ('a' - 10);
        else return NULL;
    }

    return p;
}

/*  码点范围        码点位数    byte1           byte2           byte3           byte4
 * 0x0000-0x007F      7      0xxxxxxx
 * 0x0080-0x07ff      11     110xxxxx         10xxxxxx
 * 0x0800-0xffff      16     1110xxxx         10xxxxxx        10xxxxxx
 * 0x10000-0x10ffff   21     11110xxx         10xxxxxx        10xxxxxx        10xxxxxx
 */
static void json_encode_utf8(json_context *context, unsigned unicode) {
    assert(0x0000 <= unicode  && unicode <= 0x10ffff);

    if (unicode <= 0x007f) {
        PUTC(context, unicode);
    } else if (unicode <= 0x07ff) {
        PUTC(context, 0xC0 | unicode >> 6 & 0xFF);
        PUTC(context, 0x80 | unicode & 0x3F);
    }else if (unicode <= 0xffff) {
        PUTC(context, 0xE0 | unicode >> 12 & 0xFF);
        PUTC(context, 0x80 | unicode >> 6 & 0x3F);
        PUTC(context, 0x80 | unicode & 0x3F);
    } else  {
        PUTC(context, 0xF0 | (unicode >> 18 & 0xFF) );
        PUTC(context, 0x80 | (unicode >> 12 & 0x3F) );
        PUTC(context, 0x80 | (unicode >> 6 & 0x3F) );
        PUTC(context, 0x80 | (unicode & 0x3F) );
    }
}

static int json_parse_string(json_context *context, json_value *value) {
    size_t head = context->top, len;
    unsigned u;
    const char *p;
    EXPECT(context, '\"');

    p = context->json;

    for (;;) {
        char ch = *p++;
        switch (ch) {
            case '\"':
                len = context->top - head;
                json_set_string(value, (const char *)json_context_pop(context, len), len);
                return JSON_PARSE_OK;
            case '\0':
                STRING_PARSE_ERR(JSON_PARSE_MISS_QUOTATION_MARK);
            case '\\':
                switch (*p++) {
                    case '\\': PUTC(context,'\\'); break;
                    case '/': PUTC(context,'/'); break;
                    case '\"': PUTC(context,'\"'); break;
                    case 'b': PUTC(context,'\b'); break;
                    case 'f': PUTC(context,'\f'); break;
                    case 'n': PUTC(context,'\n'); break;
                    case 'r': PUTC(context,'\r'); break;
                    case 't': PUTC(context,'\t'); break;
                    case 'u':
                        if (!(p = json_parse_hex(p, &u))) {
                            STRING_PARSE_ERR(JSON_PARSE_INVALID_UNICODE_HEX);
                        }
                        json_encode_utf8(context, u);
                        break;
                    default: {
                        STRING_PARSE_ERR(JSON_PARSE_MISS_QUOTATION_MARK);
                    }
                }
                break;
            default:
                if ((unsigned char)ch < 0x20) {
                    STRING_PARSE_ERR(JSON_PARSE_INVALID_STRING_CHAR);
                }
                PUTC(context, ch);
        }
    }

}

/* value = null / false / true */
static int json_parse_value(json_context *context, json_value *value) {
    switch (*context->json) {
        case 'n': return json_parse_literal(context, value, "null", JSON_NULL);
        case 't': return json_parse_literal(context, value, "true", JSON_TRUE);
        case 'f': return json_parse_literal(context, value, "false", JSON_FALSE);
        case '\"': return json_parse_string(context, value);
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

void json_set_number(json_value *value, double number) {
    assert(value != NULL);
    value_free(value);
    value->u.number = number;
    value->type = JSON_NUMBER;
}


int json_get_boolean(const json_value *value) {
    assert(value != NULL && (JSON_TRUE == value->type || JSON_FALSE == value->type ));
    return value->type == JSON_TRUE;
}

void json_set_boolean(json_value *value, int b) {
    assert(value != NULL);
    value_free(value);

    value->type = b ? JSON_TRUE : JSON_FALSE;
}


size_t json_get_string_length(const json_value *value)
{
    assert(NULL != value && JSON_STRING == value->type);
    return value->u.string.len;
}


const char* json_get_string(const json_value *value) {
    assert(NULL != value && JSON_STRING == value->type);
    return value->u.string.str;
}


void json_set_string(json_value *value, const char * s, size_t len) {
    assert(NULL != value && (NULL != s || 0 == len));

    value_free(value);

    value->u.string.str = (char *)malloc(len + 1);
    memcpy(value->u.string.str, s, len);
    value->u.string.str[len] = '\0';
    value->u.string.len = len;

    value->type = JSON_STRING;
}


int json_parse(json_value* value, const char* json) {
    json_context context;
    int ret;
    assert(value != NULL);
    context.json = json;
    context.stack = NULL;
    context.size = context.top = 0;

    json_value_init(value);
    json_parse_whitespace(&context);

    if (JSON_PARSE_OK == (ret = json_parse_value(&context, value))) {

    }

    assert(0 == context.top);
    free(context.stack);

    return ret;
}

json_type json_get_type(const json_value *value) {
    assert(value != NULL);
    return value->type;
}


