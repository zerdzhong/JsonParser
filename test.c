#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "JsonParser.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
		do {\
				test_count++;\
				if (equality) {\
						test_pass++;\
				} else { \
						 fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
						  main_ret = 1;\
				}\
		} while(0)

#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%lu")
#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%f")
#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == (alength) && memcmp(expect, actual, alength) == 0, expect, actual, "%s")

#define TEST_PARSE_TYPE(expect, json) \
    do {\
        json_value value;\
        value.type = JSON_FALSE;\
        EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&value, json));\
		EXPECT_EQ_INT(expect, json_get_type(&value));\
    } while (0)

static void test_parse() {
    TEST_PARSE_TYPE(JSON_NULL, "null");
    TEST_PARSE_TYPE(JSON_FALSE, "false");
    TEST_PARSE_TYPE(JSON_TRUE, "true");

}

#define TEST_NUMBER(expect, json) \
	do {\
		json_value value;\
		value.type = JSON_FALSE;\
		EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&value, json));\
		EXPECT_EQ_INT(JSON_NUMBER, json_get_type(&value));\
		EXPECT_EQ_DOUBLE(expect, json_get_number(&value));\
	} while(0)


static void test_parse_number() {
	TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */
}

#define TEST_ERROR(expect, json) \
    do {\
        json_value value;\
        value.type = JSON_FALSE;\
        EXPECT_EQ_INT(expect, json_parse(&value, json));\
		EXPECT_EQ_INT(JSON_NULL, json_get_type(&value));\
    } while (0)


static void test_parse_invalid_number() {
    /* invalid number */
    TEST_ERROR(JSON_PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(JSON_PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(JSON_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(JSON_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(JSON_PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(JSON_PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(JSON_PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(JSON_PARSE_INVALID_VALUE, "nan");
}

#define TEST_STRING(expect, json) \
	do {\
		json_value value;\
		value.type = JSON_FALSE;\
		EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&value, json));\
		EXPECT_EQ_INT(JSON_STRING, json_get_type(&value));\
		EXPECT_EQ_STRING(expect, json_get_string(&value), json_get_string_length(&value));\
	} while(0)


static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

static void test_access_number() {
    json_value value;
    json_value_init(&value);

    json_set_number(&value, 0.0);
    EXPECT_EQ_DOUBLE(0.0, json_get_number(&value));
    json_set_number(&value, 5.1);
    EXPECT_EQ_DOUBLE(5.1, json_get_number(&value));
}

static void test_access_string() {
    json_value value;
    json_value_init(&value);

    json_set_string(&value, "", 0);
    EXPECT_EQ_STRING("", json_get_string(&value), 0);
    json_set_string(&value, "test", 4);
    EXPECT_EQ_STRING("test", json_get_string(&value), 4);

    EXPECT_EQ_SIZE_T((size_t)4 ,json_get_string_length(&value));

    json_set_null(&value);
}

static void test_access_boolean() {
    json_value value;
    json_value_init(&value);

    json_set_boolean(&value, 1);
    EXPECT_EQ_INT(1, json_get_boolean(&value));
    json_set_boolean(&value, 0);
    EXPECT_EQ_INT(0, json_get_boolean(&value));
}

static void test_parse_invalid_unicode_hex() {
    TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
}

static void test_parse_array() {
    json_value value;

    json_value_init(&value);

    EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&value, "[\"abc\",[1,2,3],4]"));
    EXPECT_EQ_INT(JSON_ARRAY, json_get_type(&value));
    EXPECT_EQ_SIZE_T((size_t)3, json_get_array_size(&value));
    EXPECT_EQ_DOUBLE((double)4, json_get_number(json_get_array_element(&value, 2)));

    json_value_free(&value);


    json_value_init(&value);

    EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&value, "[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ_INT(JSON_ARRAY, json_get_type(&value));
    EXPECT_EQ_SIZE_T((size_t)5, json_get_array_size(&value));

    EXPECT_EQ_INT(JSON_NULL, json_get_type(json_get_array_element(&value, 0)));
    EXPECT_EQ_INT(JSON_FALSE, json_get_type(json_get_array_element(&value, 1)));
    EXPECT_EQ_INT(JSON_TRUE, json_get_type(json_get_array_element(&value, 2)));
    EXPECT_EQ_DOUBLE((double)123, json_get_number(json_get_array_element(&value, 3)));
    EXPECT_EQ_STRING("abc",
                     json_get_string(json_get_array_element(&value, 4)),
                     json_get_string_length(json_get_array_element(&value, 4)));

    json_value_free(&value);

    json_value_init(&value);

    EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&value, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ_INT(JSON_ARRAY, json_get_type(&value));
    EXPECT_EQ_SIZE_T((size_t)4, json_get_array_size(&value));

    EXPECT_EQ_INT(JSON_ARRAY, json_get_type(json_get_array_element(&value, 0)));
    EXPECT_EQ_SIZE_T((size_t)0, json_get_array_size(json_get_array_element(&value, 0)));

    EXPECT_EQ_INT(JSON_ARRAY, json_get_type(json_get_array_element(&value, 1)));
    EXPECT_EQ_SIZE_T((size_t)1, json_get_array_size(json_get_array_element(&value, 1)));
    EXPECT_EQ_DOUBLE((double)0, json_get_number(json_get_array_element(json_get_array_element(&value, 1),0)));

    EXPECT_EQ_INT(JSON_ARRAY, json_get_type(json_get_array_element(&value, 2)));
    EXPECT_EQ_SIZE_T((size_t)2, json_get_array_size(json_get_array_element(&value, 2)));
    EXPECT_EQ_DOUBLE((double)0, json_get_number(json_get_array_element(json_get_array_element(&value, 2),0)));
    EXPECT_EQ_DOUBLE((double)1, json_get_number(json_get_array_element(json_get_array_element(&value, 2),1)));

    EXPECT_EQ_INT(JSON_ARRAY, json_get_type(json_get_array_element(&value, 3)));
    EXPECT_EQ_SIZE_T((size_t)3, json_get_array_size(json_get_array_element(&value, 3)));
    EXPECT_EQ_DOUBLE((double)0, json_get_number(json_get_array_element(json_get_array_element(&value, 3),0)));
    EXPECT_EQ_DOUBLE((double)1, json_get_number(json_get_array_element(json_get_array_element(&value, 3),1)));
    EXPECT_EQ_DOUBLE((double)2, json_get_number(json_get_array_element(json_get_array_element(&value, 3),2)));


    json_value_free(&value);
}

static void test_parse_miss_key() {
    TEST_ERROR(JSON_PARSE_MISS_KEY, "{:1,");
    TEST_ERROR(JSON_PARSE_MISS_KEY, "{1:1,");
    TEST_ERROR(JSON_PARSE_MISS_KEY, "{true:1,");
    TEST_ERROR(JSON_PARSE_MISS_KEY, "{false:1,");
    TEST_ERROR(JSON_PARSE_MISS_KEY, "{null:1,");
    TEST_ERROR(JSON_PARSE_MISS_KEY, "{[]:1,");
    TEST_ERROR(JSON_PARSE_MISS_KEY, "{{}:1,");
    TEST_ERROR(JSON_PARSE_MISS_KEY, "{\"a\":1,");
}

static void test_parse_miss_colon() {
    TEST_ERROR(JSON_PARSE_MISS_COLON, "{\"a\"}");
    TEST_ERROR(JSON_PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket() {
    TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
    TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
    TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
    TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

static void test_parse_object() {
    json_value v;
    size_t i;

    json_value_init(&v);
    EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&v, " { \"num\" : 12 } "));
    EXPECT_EQ_INT(JSON_OBJECT, json_get_type(&v));
    EXPECT_EQ_SIZE_T((size_t)1, json_get_object_size(&v));
    EXPECT_EQ_STRING("num", json_get_object_key(&v, 0), json_get_object_key_length(&v, 0));
    json_value_free(&v);

    json_value_init(&v);
    EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&v,
                                            " { "
                                            "\"n\" : null , "
                                            "\"f\" : false , "
                                            "\"t\" : true , "
                                            "\"i\" : 123 , "
                                            "\"s\" : \"abc\", "
                                            "\"a\" : [ 1, 2, 3 ],"
                                            "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
                                            " } "
    ));
    EXPECT_EQ_INT(JSON_OBJECT, json_get_type(&v));
    EXPECT_EQ_SIZE_T((size_t)7, json_get_object_size(&v));
    EXPECT_EQ_STRING("n", json_get_object_key(&v, 0), json_get_object_key_length(&v, 0));
    EXPECT_EQ_INT(JSON_NULL,   json_get_type(json_get_object_value(&v, 0)));
    EXPECT_EQ_STRING("f", json_get_object_key(&v, 1), json_get_object_key_length(&v, 1));
    EXPECT_EQ_INT(JSON_FALSE,  json_get_type(json_get_object_value(&v, 1)));
    EXPECT_EQ_STRING("t", json_get_object_key(&v, 2), json_get_object_key_length(&v, 2));
    EXPECT_EQ_INT(JSON_TRUE,   json_get_type(json_get_object_value(&v, 2)));
    EXPECT_EQ_STRING("i", json_get_object_key(&v, 3), json_get_object_key_length(&v, 3));
    EXPECT_EQ_INT(JSON_NUMBER, json_get_type(json_get_object_value(&v, 3)));
    EXPECT_EQ_DOUBLE(123.0, json_get_number(json_get_object_value(&v, 3)));
    EXPECT_EQ_STRING("s", json_get_object_key(&v, 4), json_get_object_key_length(&v, 4));
    EXPECT_EQ_INT(JSON_STRING, json_get_type(json_get_object_value(&v, 4)));
    EXPECT_EQ_STRING("abc", json_get_string(json_get_object_value(&v, 4)), json_get_string_length(json_get_object_value(&v, 4)));
    EXPECT_EQ_STRING("a", json_get_object_key(&v, 5), json_get_object_key_length(&v, 5));
    EXPECT_EQ_INT(JSON_ARRAY, json_get_type(json_get_object_value(&v, 5)));
    EXPECT_EQ_SIZE_T((size_t)3, json_get_array_size(json_get_object_value(&v, 5)));
    for (i = 0; i < 3; i++) {
        json_value* e = json_get_array_element(json_get_object_value(&v, 5), i);
        EXPECT_EQ_INT(JSON_NUMBER, json_get_type(e));
        EXPECT_EQ_DOUBLE(i + 1.0, json_get_number(e));
    }
    EXPECT_EQ_STRING("o", json_get_object_key(&v, 6), json_get_object_key_length(&v, 6));
    {
        json_value* o = json_get_object_value(&v, 6);
        EXPECT_EQ_INT(JSON_OBJECT, json_get_type(o));
        for (i = 0; i < 3; i++) {
            json_value* ov = json_get_object_value(o, i);
            EXPECT_TRUE('1' + i == json_get_object_key(o, i)[0]);
            EXPECT_EQ_SIZE_T((size_t)1, json_get_object_key_length(o, i));
            EXPECT_EQ_INT(JSON_NUMBER, json_get_type(ov));
            EXPECT_EQ_DOUBLE(i + 1.0, json_get_number(ov));
        }
    }
    json_value_free(&v);
}

#define TEST_JSON_STRINGIFY(json) \
    do {\
        json_value v;\
        size_t length = 0;\
        char *json2;\
        json_value_init(&v);\
        EXPECT_EQ_INT(JSON_PARSE_OK, json_parse(&v, json));\
        json2 = json_stringify(&v, &length);\
        EXPECT_EQ_STRING(json, json2, length);\
        json_value_free(&v);\
        free(json2);\
    } while(0);

static void testJsonStringify()
{
    TEST_JSON_STRINGIFY("null");
    TEST_JSON_STRINGIFY("true");
    TEST_JSON_STRINGIFY("false");

    TEST_JSON_STRINGIFY("0");
    TEST_JSON_STRINGIFY("-1");
    TEST_JSON_STRINGIFY("1.5");
    TEST_JSON_STRINGIFY("-1.5");

    TEST_JSON_STRINGIFY("\"\"");
    TEST_JSON_STRINGIFY("\"Hello\"");
    TEST_JSON_STRINGIFY("\"Hello\\nWorld\"");
    TEST_JSON_STRINGIFY("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
    TEST_JSON_STRINGIFY("\"Hello\\u0000World\"");
}

int main(int argc, char const *argv[]) {
	test_parse();
	test_parse_number();
	test_parse_invalid_number();
    test_parse_string();
    test_parse_array();
    test_parse_object();

    test_access_number();
    test_access_string();
    test_access_boolean();

    test_parse_invalid_unicode_hex();

    test_parse_miss_key();
    test_parse_miss_colon();
    test_parse_miss_comma_or_curly_bracket();

    testJsonStringify();

    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
}
