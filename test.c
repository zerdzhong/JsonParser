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
#if 0
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
#endif
}

int main(int argc, char const *argv[]) {
	test_parse();
	test_parse_number();
	test_parse_invalid_number();

    test_parse_string();

    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
}
