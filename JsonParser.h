#ifndef JSON_PARSER_H_
#define JSON_PARSER_H_

#include <stdlib.h>

typedef enum {
	JSON_NULL,
	JSON_FALSE,
	JSON_TRUE,
	JSON_NUMBER,
	JSON_STRING,
	JSON_ARRAY,
	JSON_OBJECT
} json_type;


typedef struct json_value json_value;
typedef struct json_member json_member;

struct json_value {
    union {
        struct {json_member *member; size_t size;}object;
        struct {json_value *value; size_t size;} array;
        struct {char *str; size_t len;} string;
        double number;
    } u;

	json_type type;
};

struct json_member {
    char *key; size_t key_len; /* member key string, key string length */
    json_value value;
};

enum {
	JSON_PARSE_OK = 0,
	JSON_PARSE_EXPECT_VALUE,
	JSON_PARSE_INVALID_VALUE,
	JSON_PARSE_MISS_QUOTATION_MARK,
    JSON_PARSE_INVALID_STRING_CHAR,
    JSON_PARSE_INVALID_UNICODE_HEX,
	JSON_PARSE_INVALID_UNICODE_SURROGATE,
    JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    JSON_PARSE_MISS_KEY,
    JSON_PARSE_MISS_COLON,
    JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET
};

#define json_value_init(v) do {(v)->type = JSON_NULL;} while(0)
#define json_set_null(v) do {json_value_free((v));} while(0)


int json_parse(json_value* v, const char* json);
json_type json_get_type(const json_value *value);

void json_value_free(json_value *value);

double json_get_number(const json_value *value);
void json_set_number(json_value *value, double number);

int json_get_boolean(const json_value *value);
void json_set_boolean(json_value *value, int b);

size_t json_get_string_length(const json_value *value);
const char* json_get_string(const json_value *value);
void json_set_string(json_value *value, const char * s, size_t len);

json_value* json_get_array_element(const json_value *value, unsigned index);
size_t json_get_array_size(const json_value *value);

size_t json_get_object_size(const json_value *value);
const char* json_get_object_key(const json_value *value, unsigned index);
size_t json_get_object_key_length(const json_value *value, unsigned index);
json_value* json_get_object_value(const json_value *value, unsigned index);


#endif
