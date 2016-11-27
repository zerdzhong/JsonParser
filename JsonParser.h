#ifndef JSON_PARSER_H_
#define JSON_PARSER_H_

typedef enum {
	JSON_NULL,
	JSON_FALSE,
	JSON_TRUE,
	JSON_NUMBER,
	JSON_STRING,
	JSON_ARRAY,
	JSON_OBJECT
} json_type

typedef struct {
	json_type type;
} json_value

enum  {
	JSON_PARSE_OK = 0,
	JSON_PARSE_EXPECT_VAVLUE,
	JSON_PASE_INVALID_VALUE,
	JSON_PARSE_ROOT_NOT_SINGULAR
};

int json_parse(json_value* v, const char* json);

#endif
