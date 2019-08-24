#ifndef json_builder_h
#define json_builder_h

#define json(buf, ...) build_json(buf, sizeof(buf), __VA_ARGS__, NULL)  // returns json length if all good, negative number if error

#define JSON_ERR_BUF_SIZE -1

#include <stdarg.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int build_json(char* json, size_t buf_size, const char* item, ...);
int vbuild_json(char* json, size_t buf_size, const char* item, va_list args);

#ifdef __cplusplus
}
#endif

#endif  // json_builder_h
