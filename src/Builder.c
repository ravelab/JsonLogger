#include "JsonLogger.h"

#define json_cat_safe(src, src_size)        \
  do {                                      \
    if (json_len + src_size > buf_size) {   \
      return JSON_ERR_BUF_SIZE;             \
    }                                       \
    memcpy(&json[json_len], src, src_size); \
    json_len += src_size - 1;               \
  } while (0)

#define json_cat_array(src) json_cat_safe(src, sizeof(src))
#define json_cat_pointer(src) json_cat_safe(src, strlen(src) + 1)

#define addKey(key)        \
  do {                     \
    json_cat_pointer(key); \
    json_cat_array("\":"); \
  } while (0)

#define addInt(value)                 \
  do {                                \
    sprintf(temp, "%" PRId32, value); \
    json_cat_pointer(temp);           \
  } while (0)

#define addDouble(value, precisionChar)                                                     \
  do {                                                                                      \
    uint8_t digits = precisionChar >= 'a' ? precisionChar - 'a' + 10 : precisionChar - '0'; \
    if (digits > 17) {                                                                      \
      digits = 17;                                                                          \
    }                                                                                       \
    sprintf(temp, "%.*g", digits, value);                                                   \
    json_cat_pointer(temp);                                                                 \
  } while (0)

#define addBool(value)         \
  do {                         \
    if (value) {               \
      json_cat_array("true");  \
    } else {                   \
      json_cat_array("false"); \
    }                          \
  } while (0)

#define addOther(value)        \
  do {                         \
    if (value) {               \
      json_cat_pointer(value); \
    } else {                   \
      json_cat_array("null");  \
    }                          \
  } while (0)

#define addStr(value)                                         \
  do {                                                        \
    char* escaped1 = str_replace((char*)value, "\\", "\\\\"); \
    char* source1 = escaped1 ? escaped1 : (char*)value;       \
    char* escaped2 = str_replace(source1, "\n", "\\n");       \
    char* source2 = escaped2 ? escaped2 : source1;            \
    char* escaped3 = str_replace(source2, "\"", "\\\"");      \
    char* source3 = escaped3 ? escaped3 : source2;            \
    char* escaped4 = str_replace(source3, "\b", "\\b");       \
    char* source4 = escaped4 ? escaped4 : source3;            \
    char* escaped5 = str_replace(source4, "\f", "\\f");       \
    char* source5 = escaped5 ? escaped5 : source4;            \
    char* escaped6 = str_replace(source5, "\r", "\\r");       \
    char* source6 = escaped6 ? escaped6 : source5;            \
    char* escaped7 = str_replace(source6, "\t", "\\t");       \
    char* source7 = escaped7 ? escaped7 : source6;            \
    json_cat_pointer(source7);                                \
    if (escaped1) {                                           \
      free(escaped1);                                         \
    }                                                         \
    if (escaped2) {                                           \
      free(escaped2);                                         \
    }                                                         \
    if (escaped3) {                                           \
      free(escaped3);                                         \
    }                                                         \
    if (escaped4) {                                           \
      free(escaped4);                                         \
    }                                                         \
    if (escaped5) {                                           \
      free(escaped5);                                         \
    }                                                         \
    if (escaped6) {                                           \
      free(escaped6);                                         \
    }                                                         \
    if (escaped7) {                                           \
      free(escaped7);                                         \
    }                                                         \
  } while (0)

enum ArrayType {
  NOT_ARRAY,
  INT_ARRAY,
  DOUBLE_ARRAY,
  BOOL_ARRAY,
  STRING_ARRAY,
  OTHER_ARRAY,
};

// inspired by https://stackoverflow.com/questions/779875/what-is-the-function-to-replace-string-in-c
// You must free the result if result is non-NULL.
char* str_replace(char* orig, const char* rep, const char* with) {
  char* result;   // the return string
  char* ins;      // the next insert point
  char* tmp;      // varies
  int len_rep;    // length of rep (the string to remove)
  int len_with;   // length of with (the string to replace rep with)
  int len_front;  // distance between rep and end of last rep
  int count;      // number of replacements
  int first;

  // sanity checks and initialization
  if (!orig || !rep)
    return NULL;
  len_rep = strlen(rep);
  if (len_rep == 0)
    return NULL;  // empty rep causes infinite loop during count
  if (!with)
    with = "";
  len_with = strlen(with);

  // count the number of replacements needed
  if (!(ins = strstr(orig, rep))) {
    return NULL;  // no replacing needed
  }

  int expand = len_with > len_rep;

  if (expand) {
    char* moreIns = ins + len_rep;
    for (count = 1; (tmp = strstr(moreIns, rep)); ++count) {
      moreIns = tmp + len_rep;
    }
    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);
    if (!result)
      return NULL;
  } else {  // inplace
    tmp = ins;
    result = NULL;
    first = 1;
  }

  // first time through the loop, all the variable are set correctly
  // from here on,
  //    tmp points to the end of the result string
  //    ins points to the next occurrence of rep in orig
  //    orig points to the remainder of orig after "end of rep"
  do {
    len_front = ins - orig;
    if (expand) {
      tmp = memcpy(tmp, orig, len_front) + len_front;
      tmp = memcpy(tmp, with, len_with) + len_with;
    } else {
      if (first) {
        first = 0;
      } else {
        tmp = memmove(tmp, orig, len_front) + len_front;
      }
      tmp = memmove(tmp, with, len_with) + len_with;
    }
    orig += len_front + len_rep;  // move to next "end of rep"
  } while ((ins = strstr(orig, rep)));

  if (expand) {
    strcpy(tmp, orig);
  } else {
    memmove(tmp, orig, strlen(orig) + 1);
  }
  return result;
}

int build_json(char* json, size_t buf_size, const char* item, ...) {
  va_list args;
  va_start(args, item);
  int ret = vbuild_json(json, buf_size, item, args);
  va_end(args);
  return ret;
}

int vbuild_json(char* json, size_t buf_size, const char* item, va_list arg) {
  if (!json) {
    return JSON_ERR_BUF_SIZE;
  }
  json[0] = '\0';
  int json_len = 0;
  int8_t firstItem = 1;
  int8_t bracket = 1;
  enum ArrayType array = NOT_ARRAY;
  int32_t numOfArrayItems;
  char doubleArrayPrecisionChar = 0;
  char temp[25];  // enough for int, double conversion

  if (item[0] == '-' && item[1] == '{') {
    bracket = 0;
    item = va_arg(arg, const char*);
  } else if (item[1] == '[') {
    switch (item[0]) {
      case 'i':
        array = INT_ARRAY;
        break;
      case 'f':
        array = DOUBLE_ARRAY;
        doubleArrayPrecisionChar = item[2];
        break;
      case 'b':
        array = BOOL_ARRAY;
        break;
      case 'o':
        array = OTHER_ARRAY;
        break;
      case 's':
        array = STRING_ARRAY;
        break;
    }
    if (array != NOT_ARRAY) {
      numOfArrayItems = va_arg(arg, int32_t);
    }
  }

  if (array == NOT_ARRAY) {
    int8_t lastValueNeedsQuote = 0;

    while (item) {
      int8_t isKey = (item[0] != '+' || item[1] != '|');
      if (firstItem) {
        if (isKey) {
          if (bracket) {
            json_cat_array("{\"");
          } else {
            json_cat_array("+|\"");
          }
        } else {
          json_cat_array("{");
        }
        firstItem = 0;
      } else {
        if (lastValueNeedsQuote) {
          json_cat_array("\",\"");
          lastValueNeedsQuote = 0;
        } else {
          if (isKey) {
            json_cat_array(",\"");
          } else {
            json_cat_array(",");
          }
        }
      }
      if (item[1] == '|' && item[0] == 'i') {  // integer
        addKey(&item[2]);
        int32_t value = va_arg(arg, int32_t);
        addInt(value);
      } else if (item[1] == '|' && item[0] == 'f') {  // double
        addKey(&item[3]);
        double value = va_arg(arg, double);
        addDouble(value, item[2]);
      } else if (item[1] == '|' && item[0] == 'b') {  // boolean
        addKey(&item[2]);
        int32_t value = va_arg(arg, int32_t);
        addBool(value);
      } else if (item[1] == '|' && item[0] == 'o') {  // others "{object}", "[array]", "null"
        addKey(&item[2]);
        const char* value = va_arg(arg, const char*);
        addOther(value);
      } else if (item[1] == '|' && item[0] == '+') {  // insert fragment
        json_cat_pointer(&item[2]);
      } else {  // string
        int itemIsValue = 0;
        const char* value = va_arg(arg, const char*);
        if (!value) {
          value = item;
          json_cat_array(EMPTY_KEY "\":\"");
          itemIsValue = 1;
        } else {
          char* key = (char*)((item[1] == '|' && item[0] == 's') ? item + 2 : item);
          json_cat_pointer(key);
          json_cat_array("\":\"");
        }
        addStr(value);
        lastValueNeedsQuote = 1;
        if (itemIsValue) {
          break;
        }
      }
      item = va_arg(arg, const char*);
    }

    if (bracket) {
      if (lastValueNeedsQuote) {
        json_cat_array("\"}");
      } else {
        json_cat_array("}");
      }
    } else {
      if (lastValueNeedsQuote) {
        json_cat_array("\"");
      }
    }
  } else {  // ARRAY
    int8_t arrayItemsNeedsQuote = 0;
    if (numOfArrayItems == 0) {
      json_cat_array("[]");
      return 2;
    }

    void** list;
    switch (array) {
      case INT_ARRAY: {
        list = (void**)va_arg(arg, int32_t*);
        break;
      }
      case DOUBLE_ARRAY: {
        list = (void**)va_arg(arg, double*);
        break;
      }
      case BOOL_ARRAY: {
        list = (void**)va_arg(arg, int32_t*);
        break;
      }
      case OTHER_ARRAY: {
        list = (void**)va_arg(arg, const char**);
        break;
      }
      case STRING_ARRAY: {
        list = (void**)va_arg(arg, const char**);
        break;
      }
      default:
        break;
    }

    for (int32_t i = 0; i < numOfArrayItems; i++) {
      if (firstItem) {
        if (array == STRING_ARRAY) {
          json_cat_array("[\"");
          arrayItemsNeedsQuote = 1;
        } else {
          json_cat_array("[");
        }
        firstItem = 0;
      } else {
        if (arrayItemsNeedsQuote) {
          json_cat_array("\",\"");
        } else {
          json_cat_array(",");
        }
      }
      switch (array) {
        case INT_ARRAY: {
          addInt(((int32_t*)list)[i]);
          break;
        }
        case DOUBLE_ARRAY: {
          addDouble(((double*)list)[i], doubleArrayPrecisionChar);
          break;
        }
        case BOOL_ARRAY: {
          addBool(((int32_t*)list)[i]);
          break;
        }
        case OTHER_ARRAY: {
          addOther(((const char**)list)[i]);
          break;
        }
        case STRING_ARRAY: {
          addStr(((const char**)list)[i]);
          break;
        }
        default:
          break;
      }
    }

    if (arrayItemsNeedsQuote) {
      json_cat_array("\"]");
    } else {
      json_cat_array("]");
    }
  }
  return json_len;
}

#ifdef JSON_BUILDER_TEST
// gcc -Os -DJSON_BUILDER_TEST src/*.c; ./a.out; rm ./a.out

#include <assert.h>

int main() {
  int len;
  char buf3[3], buf256[256], buf64[64];

  len = json(buf256, "str_key1", "str1", "i|int_key1", 7, "f|3double_key1", 3.14159,
             "b|boolean_key1", 1, "o|object_key1", "{}", "o|array_key1", "[]", "o|null_key1", "null");
  printf("%s\n", buf256);
  assert(!strcmp(buf256, "{\"str_key1\":\"str1\",\"int_key1\":7,\"double_key1\":3.14,\"boolean_key1\":true,\"object_key1\":{},\"array_key1\":[],\"null_key1\":null}"));
  assert(len == strlen(buf256));

  len = json(buf64, "-{", "str_key2", "str2", "i|int_key2", 8);
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "+|\"str_key2\":\"str2\",\"int_key2\":8"));
  assert(len == strlen(buf64));

  char* buf512 = (char*)malloc(512);
  len = jsonHeap(buf512, 512, "o|obj", buf256, buf64);
  printf("%s\n", buf512);
  assert(!strcmp(buf512, "{\"obj\":{\"str_key1\":\"str1\",\"int_key1\":7,\"double_key1\":3.14,\"boolean_key1\":true,\"object_key1\":{},\"array_key1\":[],\"null_key1\":null},\"str_key2\":\"str2\",\"int_key2\":8}"));
  assert(len == strlen(buf512));
  free(buf512);

  len = json(buf256, "value only");
  printf("%s\n", buf256);
  assert(!strcmp(buf256, "{\"_\":\"value only\"}"));
  assert(len == strlen(buf256));

  char* strArray[] = {"str3", "str4\"inquote\""};
  len = json(buf64, "s[", 2, strArray);
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "[\"str3\",\"str4\\\"inquote\\\"\"]"));
  assert(len == strlen(buf64));

  int32_t intArray[] = {0, -2147483648, 2147483647};
  len = json(buf64, "i[", 3, intArray);
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "[0,-2147483648,2147483647]"));
  assert(len == strlen(buf64));

  double floatArray[] = {0.0, 0.01, 4.44, 1.2345678901234567890};
  len = json(buf64, "f[1", 4, floatArray);
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "[0,0.01,4,1]"));
  assert(len == strlen(buf64));

  // up to 17 significant number, the valid values for the # in "f[#" are 1 to h (a is 10, b is 11 ... h is 17)
  len = json(buf64, "f[7", 4, floatArray);
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "[0,0.01,4.44,1.234568]"));
  assert(len == strlen(buf64));

  double floatArray2[] = {0.0, 0.01, 4.44, -1.2345678901234567890, -0x1.fffffffffffffp+1023, -2.2250738585072014e-308};
  len = json(buf256, "f[h", 6, floatArray2);
  printf("%s\n", buf256);
  assert(!strcmp(buf256, "[0,0.01,4.4400000000000004,-1.2345678901234567,-1.7976931348623157e+308,-2.2250738585072014e-308]"));
  assert(len == strlen(buf256));

  int32_t boolArray[] = {0, 1};
  len = json(buf64, "b[", 2, boolArray);
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "[false,true]"));
  assert(len == strlen(buf64));

  char* otherArray[] = {"[]", "{}", "null", "40", "5.55", "false", "\"str5\""};
  len = json(buf64, "o[", 7, otherArray);
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "[[],{},null,40,5.55,false,\"str5\"]"));
  assert(len == strlen(buf64));

  len = json(buf3, "o[", 0);
  printf("%s\n", buf3);
  assert(!strcmp(buf3, "[]"));
  assert(len == strlen(buf3));

  strArray[0] = buf64;
  strArray[1] = buf3;
  len = json(buf256, "s[", 2, strArray);
  printf("%s\n", buf256);
  assert(!strcmp(buf256, "[\"[[],{},null,40,5.55,false,\\\"str5\\\"]\",\"[]\"]"));
  assert(len == strlen(buf256));

  len = json(buf64, "\\\n\b\t\r\f\"");
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "{\"_\":\"\\\\\\n\\b\\t\\r\\f\\\"\"}"));
  assert(len == strlen(buf64));

  // corner cases

  len = json(buf64, "");
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "{\"_\":\"\"}"));
  assert(len == strlen(buf64));

  len = json(buf64, "a", "");
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "{\"a\":\"\"}"));
  assert(len == strlen(buf64));

  len = json(buf64, "x|");
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "{\"_\":\"x|\"}"));
  assert(len == strlen(buf64));

  len = json(buf64, "x|", "value");
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "{\"x|\":\"value\"}"));
  assert(len == strlen(buf64));

  len = json(buf64, "s|i|", "value");
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "{\"i|\":\"value\"}"));
  assert(len == strlen(buf64));

  len = json(buf64, "x[");
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "{\"_\":\"x[\"}"));
  assert(len == strlen(buf64));

  // error conditions
  len = json(NULL, "k", "v");
  assert(len == JSON_ERR_BUF_SIZE);

  len = json(buf64, "01234567890123456789012345678901234567890123456789012345678901234567890123456789", "v");
  printf("%s\n", buf64);
  assert(len == JSON_ERR_BUF_SIZE);

  len = json(buf64, "k", "01234567890123456789012345678901234567890123456789012345678901234567890123456789");
  printf("%s\n", buf64);
  assert(len == JSON_ERR_BUF_SIZE);

  // len = json(buf64, "i|");
  // printf("%s\n", buf64);
  // assert(!strcmp(buf64, "{\"\":0}"));
  // assert(len == strlen(buf64));

  // compile with -O2 gets segmentation fault 11
  // len = json(buf64, "f|2");
  // printf("%s\n", buf64);  // gonna be random value converted from 64-bit number

  // len = json(buf64, "b|");
  // printf("%s\n", buf64);
  // assert(!strcmp(buf64, "{\"\":false}"));
  // assert(len == strlen(buf64));

  // len = json(buf64, "o|");
  // printf("%s\n", buf64);
  // assert(!strcmp(buf64, "{\"\":null}"));
  // assert(len == strlen(buf64));

  return 0;
}
#endif