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

#define EXPECT_EQ_UNSIGNED(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%lu")
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

    EXPECT_EQ_UNSIGNED((size_t)4 ,json_get_string_length(&value));

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
    EXPECT_EQ_UNSIGNED((size_t)3, json_get_array_size(&value));
    EXPECT_EQ_DOUBLE((double)4, json_get_number(json_get_array_element(&value, 2)));

    json_value_free(&value);
}

int main(int argc, char const *argv[]) {
	test_parse();
	test_parse_number();
	test_parse_invalid_number();
    test_parse_string();
    test_parse_array();

    test_access_number();
    test_access_string();
    test_access_boolean();

    test_parse_invalid_unicode_hex();

    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
}
