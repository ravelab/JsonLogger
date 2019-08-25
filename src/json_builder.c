#include "json_builder.h"

#include <stdlib.h>
#include <string.h>

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

#define addInt()                  \
  do {                            \
    int value = va_arg(arg, int); \
    sprintf(temp, "%i", value);   \
    json_cat_pointer(temp);       \
  } while (0)

#define addDouble(precisionChar)                                                            \
  do {                                                                                      \
    double value = va_arg(arg, double);                                                     \
    uint8_t digits = precisionChar >= 'a' ? precisionChar - 'a' + 10 : precisionChar - '0'; \
    if (digits > 17) {                                                                      \
      digits = 17;                                                                          \
    }                                                                                       \
    sprintf(temp, "%.*g", digits, value);                                                   \
    json_cat_pointer(temp);                                                                 \
  } while (0)

#define addBool()                 \
  do {                            \
    int value = va_arg(arg, int); \
    if (value) {                  \
      json_cat_array("true");     \
    } else {                      \
      json_cat_array("false");    \
    }                             \
  } while (0)

#define addOther()                                \
  do {                                            \
    const char* value = va_arg(arg, const char*); \
    if (value) {                                  \
      json_cat_pointer(value);                    \
    } else {                                      \
      json_cat_array("null");                     \
    }                                             \
  } while (0)

#define addStr(value)                                 \
  do {                                                \
    char* escaped = str_replace(value, "\"", "\\\""); \
    char* source = escaped ? escaped : (char*)value;  \
    json_cat_pointer(source);                         \
    if (escaped) {                                    \
      free(escaped);                                  \
    }                                                 \
  } while (0)

enum ArrayType {
  NOT_ARRAY,
  INT_ARRAY,
  DOUBLE_ARRAY,
  BOOL_ARRAY,
  STRING_ARRAY,
  OTHER_ARRAY,
};

// Modified from https://stackoverflow.com/questions/779875/what-is-the-function-to-replace-string-in-c
// You must free the result if result is non-NULL.
static char* str_replace(const char* orig, char* rep, char* with) {
  char* result;   // the return string
  char* ins;      // the next insert point
  char* tmp;      // varies
  int len_rep;    // length of rep (the string to remove)
  int len_with;   // length of with (the string to replace rep with)
  int len_front;  // distance between rep and end of last rep
  int count;      // number of replacements

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
  ins = (char*)orig;
  for (count = 0; (tmp = strstr(ins, rep)); ++count) {
    ins = tmp + len_rep;
  }

  if (!count)
    return NULL;  // no replacing needed

  tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

  if (!result)
    return NULL;

  // first time through the loop, all the variable are set correctly
  // from here on,
  //    tmp points to the end of the result string
  //    ins points to the next occurrence of rep in orig
  //    orig points to the remainder of orig after "end of rep"
  while (count--) {
    ins = strstr(orig, rep);
    len_front = ins - orig;
    tmp = strncpy(tmp, orig, len_front) + len_front;
    tmp = strcpy(tmp, with) + len_with;
    orig += len_front + len_rep;  // move to next "end of rep"
  }
  strcpy(tmp, orig);
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
  int numOfArrayItems;
  int doubleArrayPrecisionChar = 0;
  char temp[32];  // should be enough for int, double conversion

  if (item[0] == '-' && item[1] == '{') {
    bracket = 0;
    item = va_arg(arg, const char*);
  } else if (item[1] == '[') {
    switch (item[0]) {
      case 'i':
        array = INT_ARRAY;
        break;
      case 'd':
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
      numOfArrayItems = va_arg(arg, int);
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
      if (item[1] == '|') {
        switch (item[0]) {
          case 'i': {  // integer
            addKey(&item[2]);
            addInt();
            break;
          }
          case 'd': {  // double
            addKey(&item[3]);
            addDouble(item[2]);
            break;
          }
          case 'b': {  // boolean
            addKey(&item[2]);
            addBool();
            break;
          }
          case 'o': {  // others "{object}", "[array]", "null"
            addKey(&item[2]);
            addOther();
            break;
          }
          case '+': {  // insert fragment
            json_cat_pointer(&item[2]);
            break;
          }
        }
      } else {  // string
        int itemIsValue = 0;
        const char* value = va_arg(arg, const char*);
        if (!value) {
          value = item;
          json_cat_array("\":\"");
          itemIsValue = 1;
        } else {
          json_cat_pointer(item);
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

    while (numOfArrayItems > 0) {
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
          addInt();
          break;
        }
        case DOUBLE_ARRAY: {
          addDouble(doubleArrayPrecisionChar);
          break;
        }
        case BOOL_ARRAY: {
          addBool();
          break;
        }
        case OTHER_ARRAY: {
          addOther();
          break;
        }
        case STRING_ARRAY: {
          const char* value = va_arg(arg, const char*);
          addStr(value);
          break;
        }
        default:
          break;
      }
      numOfArrayItems--;
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
// gcc -DJSON_BUILDER_TEST src/json_builder.c; ./a.out;

#include <assert.h>

int main() {
  int len;
  char buf3[3], buf256[256], buf64[64], buf512[512];

  len = json(buf256, "str_key1", "str1", "i|int_key1", 7, "d|3double_key1", 3.14159,
             "b|boolean_key1", 1, "o|object_key1", "{}", "o|array_key1", "[]", "o|null_key1", "null");
  printf("%s\n", buf256);
  assert(!strcmp(buf256, "{\"str_key1\":\"str1\",\"int_key1\":7,\"double_key1\":3.14,\"boolean_key1\":true,\"object_key1\":{},\"array_key1\":[],\"null_key1\":null}"));
  assert(len == strlen(buf256));

  len = json(buf64, "-{", "str_key2", "str2", "i|int_key2", 8);
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "+|\"str_key2\":\"str2\",\"int_key2\":8"));
  assert(len == strlen(buf64));

  len = json(buf512, "o|obj", buf256, buf64);
  printf("%s\n", buf512);
  assert(!strcmp(buf512, "{\"obj\":{\"str_key1\":\"str1\",\"int_key1\":7,\"double_key1\":3.14,\"boolean_key1\":true,\"object_key1\":{},\"array_key1\":[],\"null_key1\":null},\"str_key2\":\"str2\",\"int_key2\":8}"));
  assert(len == strlen(buf512));

  len = json(buf512, "value only");
  printf("%s\n", buf512);
  assert(!strcmp(buf512, "{\"\":\"value only\"}"));
  assert(len == strlen(buf512));

  len = json(buf64, "s[", 2, "str3", "str4\"inquote\"");
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "[\"str3\",\"str4\\\"inquote\\\"\"]"));
  assert(len == strlen(buf64));

  len = json(buf64, "i[", 3, 0, 10, 20);
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "[0,10,20]"));
  assert(len == strlen(buf64));

  len = json(buf64, "d[1", 4, 0.0, 0.01, 4.44, 1.2345678901234567890);
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "[0,0.01,4,1]"));
  assert(len == strlen(buf64));

  // up to 17 significant number, the valid values for the * in "d[*" are 1 to h (a is 10, b is 11 ... h is 17)
  len = json(buf64, "d[7", 4, 0.0, 0.01, 4.44, 1.2345678901234567890);
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "[0,0.01,4.44,1.234568]"));
  assert(len == strlen(buf64));

  len = json(buf64, "d[h", 4, 0.0, 0.01, 4.44, 1.2345678901234567890);
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "[0,0.01,4.4400000000000004,1.2345678901234567]"));
  assert(len == strlen(buf64));

  len = json(buf64, "b[", 2, 0, 1);
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "[false,true]"));
  assert(len == strlen(buf64));

  len = json(buf64, "o[", 7, "[]", "{}", "null", "40", "5.55", "false", "\"str5\"");
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "[[],{},null,40,5.55,false,\"str5\"]"));
  assert(len == strlen(buf64));

  len = json(buf3, "o[", 0);
  printf("%s\n", buf3);
  assert(!strcmp(buf3, "[]"));
  assert(len == strlen(buf3));

  len = json(buf512, "s[", 2, buf64, buf3);
  printf("%s\n", buf512);
  assert(!strcmp(buf512, "[\"[[],{},null,40,5.55,false,\\\"str5\\\"]\",\"[]\"]"));
  assert(len == strlen(buf512));

  // error conditions
  len = json(NULL, "k", "v");
  assert(len == JSON_ERR_BUF_SIZE);

  len = json(buf64, "01234567890123456789012345678901234567890123456789012345678901234567890123456789", "v");
  printf("%s\n", buf64);
  assert(len == JSON_ERR_BUF_SIZE);

  len = json(buf64, "k", "01234567890123456789012345678901234567890123456789012345678901234567890123456789");
  printf("%s\n", buf64);
  assert(len == JSON_ERR_BUF_SIZE);

  len = json(buf64, "b[", 2, 0, 1, 1);  // one additional parameter is ignored
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "[false,true]"));
  assert(len == strlen(buf64));

  len = json(buf64, "i|");
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "{\"\":0}"));
  assert(len == strlen(buf64));

  len = json(buf64, "d|2");
  printf("%s\n", buf64);  // gonna be random value converted from 64-bit number

  len = json(buf64, "b|");
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "{\"\":false}"));
  assert(len == strlen(buf64));

  len = json(buf64, "o|");
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "{\"\":null}"));
  assert(len == strlen(buf64));

  len = json(buf64, "");
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "{\"\":\"\"}"));
  assert(len == strlen(buf64));

  len = json(buf64, "o[", 1);
  printf("%s\n", buf64);
  assert(!strcmp(buf64, "[null]"));
  assert(len == strlen(buf64));
}
#endif