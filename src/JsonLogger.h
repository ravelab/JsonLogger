#ifndef json_builder_h
#define json_builder_h

// stuff you should know
#define json(buf, ...) build_json(buf, sizeof(buf), __VA_ARGS__, NULL)     // returns json length if all good, negative number if error
#define jsonHeap(buf, size, ...) build_json(buf, size, __VA_ARGS__, NULL)  // same as json() but user supplies buffer size for malloc-ed buffer

#define logFatal(...) logJson(LEVEL_FATAL, __VA_ARGS__)
#define logError(...) logJson(LEVEL_ERROR, __VA_ARGS__)
#define logWarn(...) logJson(LEVEL_WARN, __VA_ARGS__)
#define logInfo(...) logJson(LEVEL_INFO, __VA_ARGS__)
#define logDebug(...) logJson(LEVEL_DEBUG, __VA_ARGS__)
#define logTrace(...) logJson(LEVEL_TRACE, __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

// define LOG_ID_KEY (e.g. -D LOG_ID_KEY="i") and implement getLogId() if you want to log id
#ifdef LOG_ID_KEY
extern const char* getLogId();
#endif

// define LOG_ID_KEY (e.g. -D LOG_TIME_KEY="t") and implement getLogTime() if you want to log id
#ifdef LOG_TIME_KEY
extern const char* getLogTime();
#endif

void logAddSender(void (*sender)(int level, const char* json));
void logModifyForHuman(int level, char* json);

extern const char* LOG_LEVELS[];

#ifdef __cplusplus
}
#endif

#define JSON_ERR_BUF_SIZE -1

#ifndef EMPTY_KEY
#define EMPTY_KEY "_"
#endif

#ifndef LOG_MIN_LEVEL
#define LOG_MIN_LEVEL 1
#endif

#ifndef LOG_MAX_LEN
#define LOG_MAX_LEN 512
#endif

#ifndef LOG_LEVEL_KEY
#define LOG_LEVEL_KEY "l"
#endif

#ifndef LOG_SOURCE_KEY
#define LOG_SOURCE_KEY "s"
#endif

#ifndef LOG_FUNC_KEY
#define LOG_FUNC_KEY "f"
#endif

#define LEVEL_FATAL 5
#define LEVEL_ERROR 4
#define LEVEL_WARN 3
#define LEVEL_INFO 2
#define LEVEL_DEBUG 1
#define LEVEL_TRACE 0

// stuff you shouldn't know
#define logJson(level, ...) \
  if (level >= LOG_MIN_LEVEL) log_json(level, "", "s", SOURCE, "f", __func__, __VA_ARGS__, NULL)

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define FILELINE __FILE__ ":" TOSTRING(__LINE__)

#ifdef POSSIBLE_BACKSLASH
#define SOURCE strrchr("\\" FILELINE, '\\') + 1
#else
#define SOURCE FILELINE
#endif

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

int build_json(char* json, size_t buf_size, const char* item, ...);
int vbuild_json(char* json, size_t buf_size, const char* item, va_list args);

void log_json(int level, const char* placeholder, ...);

char* str_replace(char* orig, const char* rep, const char* with);

#ifdef __cplusplus
}
#endif

#endif  // json_builder_h
