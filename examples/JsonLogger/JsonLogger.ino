#include <JsonLogger.h>

void send_console(int level, const char* json) {
  char mod[LOG_MAX_LEN];
  strcpy(mod, json);

  logModifyForHuman(level, mod);

  Serial.println(mod);
}

void send_file(int level, const char* json) {
  if (level >= LEVEL_INFO) {
    Serial.println(json);
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
  Serial.println();
  logDebug("log to terminal, but not to file");
  Serial.println();
  logInfo("i|status", -1, "d|5pi", 3.14159, "log to both terminal and file");
  Serial.println();
  logWarn("Warning");
  Serial.println();
  logError("Error");
  Serial.println();
  logFatal("Fatal");
}

void loop() {
}
