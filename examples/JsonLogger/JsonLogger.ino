#include <JsonLogger.h>

void send_console(int level, const char* json) {
  char mod[LOG_MAX_LEN];
  strcpy(mod, json);

  str_replace(mod, "{\"" LOG_TIME_KEY "\":\"", "");
#ifdef LOG_ID_VALUE
  str_replace(mod, ",\"" LOG_ID_KEY "\":", "");
  char buf[128];
  sprintf(buf, "\"%s\"", LOG_ID_VALUE);
  str_replace(mod, buf, "");
#endif
  str_replace(mod, "\",\"" LOG_LEVEL_KEY "\":", strlen(LEVELS[level]) == 5 ? "" : " ");
  str_replace(mod, "\",\"" LOG_SOURCE_KEY "\":\"", " ");
  str_replace(mod, "\",\"" LOG_FUNC_KEY "\":\"", " ");
  str_replace(mod, "\"", " ");
  mod[strlen(mod) - 1] = '\0';

  Serial.printf("terminal: %s\n", mod);
}

void send_file(int level, const char* json) {
  if (level >= LEVEL_INFO) {
    Serial.printf("file    : %s\n", json);
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  logAddSender(send_console);
  logAddSender(send_file);

  logTrace("should not be logged at all if LOG_MIN_LEVEL is not changed to 0");
  Serial.printf("\n");
  logDebug("log to terminal, but not to file");
  Serial.printf("\n");
  logInfo("i|status", -1, "d|5pi", 3.14159, "log to both terminal and file");
  Serial.printf("\n");
  logWarn("Warning");
  Serial.printf("\n");
  logError("Error");
  Serial.printf("\n");
  logFatal("Fatal");
}

void loop() {
}
