#include "JsonLogger.h"

const char* LOG_LEVELS[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

static void (*senders[5])(int level, const char* json);
static int number_of_senders = 0;

void logAddSender(void (*sender)(int level, const char* json)) {
  senders[number_of_senders] = sender;
  number_of_senders++;
}

void logModifyForHuman(int level, char* mod) {
#ifdef LOG_TIME_KEY
  str_replace(mod, "\"" LOG_TIME_KEY "\":\"", "");
#endif

#ifdef LOG_ID_KEY
#ifdef LOG_TIME_KEY
  str_replace(mod, "\",\"" LOG_ID_KEY "\":", "");
#else
  str_replace(mod, "\"" LOG_ID_KEY "\":", "");
#endif
  char buf[128];
  sprintf(buf, "\"%s", getLogId());
  str_replace(mod, buf, "");
#endif
  str_replace(mod, "\",\"" LOG_LEVEL_KEY "\":", strlen(LOG_LEVELS[level]) == 5 ? "" : " ");
  str_replace(mod, "\",\"" LOG_SOURCE_KEY "\":\"", " ");
  str_replace(mod, "\",\"" LOG_FUNC_KEY "\":\"", " ");
  str_replace(mod, "\"", " ");
  // mod[strlen(mod) - 1] = '\0';
}

void log_json(int level, const char* placeholder, ...) {
  char fragment[LOG_MAX_LEN], json[LOG_MAX_LEN];
  json(fragment, "-{",
#ifdef LOG_TIME_KEY
       LOG_TIME_KEY, getLogTime(),
#endif
#ifdef LOG_ID_KEY
       LOG_ID_KEY, getLogId(),
#endif
       LOG_LEVEL_KEY, LOG_LEVELS[level]);
  va_list args;
  va_start(args, placeholder);
  vbuild_json(json, LOG_MAX_LEN, fragment, args);
  va_end(args);

  for (int i = 0; i < number_of_senders; i++) {
    senders[i](level, json);
  }
}

#ifdef LOGGER_TEST
// gcc -Os -DLOGGER_TEST '-DLOG_TIME_KEY="t"' src/*.c; ./a.out; rm ./a.out
// gcc -Os -DLOGGER_TEST '-DLOG_ID_KEY="i"' src/*.c; ./a.out; rm ./a.out

const char* getLogTime() {
  return "1970-01-01T00:00:00Z";
}

const char* getLogId() {
  return "DEVICE UUID";
}

void send_console(int level, const char* json) {
  char mod[LOG_MAX_LEN];
  strcpy(mod, json);

  logModifyForHuman(level, mod);

  printf("terminal: %s\n", mod);
}

void send_file(int level, const char* json) {
  if (level >= LEVEL_INFO) {
    printf("file    : %s\n", json);
  }
}

int main() {
  logAddSender(send_console);
  logAddSender(send_file);

  logTrace("should not be logged at all if LOG_MIN_LEVEL is not changed to 0");
  printf("\n");
  logDebug("log to terminal, but not to file");
  printf("\n");
  logInfo("i|status", -1, "d|5pi", 3.14159, "log to both terminal and file");
  printf("\n");
  logWarn("Warning");
  printf("\n");
  logError("Error");
  printf("\n");
  logFatal("Fatal");

  return 0;
}

#endif