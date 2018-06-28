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

typedef struct {
    union {
        struct {char *str; size_t len;} string;
        double number;
    } u;
	json_type type;
} json_value;

enum  {
	JSON_PARSE_OK = 0,
	JSON_PARSE_EXPECT_VALUE,
	JSON_PARSE_INVALID_VALUE,
	JSON_PARSE_ROOT_NOT_SINGULAR,
	JSON_PARSE_MISS_QUOTATION_MARK
};

int json_parse(json_value* v, const char* json);
json_type json_get_type(const json_value *value);

double json_get_number(const json_value *value);
void json_set_number(json_value *value, double number);

int json_get_boolean(const json_value *value);
void json_set_boolean(json_value *value, int b);

size_t json_get_string_length(const json_value *value);
const char* json_get_string(const json_value *value);
void json_set_string(json_value *value, const char * s, size_t len);


#endif
