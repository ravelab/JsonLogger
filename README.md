JSON Logger
======
An easy-to-use, small, fast and portable JSON builder and logger for IoT logging, data acquisition and analytics.

### Usage:

Special prefixes to indicate data types: 
```javascript
s| i| f#| b| o| {| }| +|
s[ i[ f#[ b[ o[
-{ 
``` 

#### JSON Builder
```c
#include <JsonLogger.h>
...
  // To build a json, call json() with a char* buffer, prefixed keys and values:
  char buf256[256], buf128[128], buf64[64];
  // key: value is a string
  // i|key: value is an integer (32 bits)
  // f#|key: a floating number with 1 to 17 significant digits (a is 10 ... h is 17).
  // b|key: value is a boolean
  // o|key: value is anything else (object, array, null)
  // {|key: object begins
  // }|: object ends
  // If the last parameter has no matching pair, it will be a value to "_" key.
  int len = json(buf256, "StrK", "StrV", "{|ObjK", "i|IntK", 0xffffffff, "f7|FloatK", 1.234567890,
                 "}|", "b|BoolK", 1, "o|NullK", "null", "ValueOnly");
  // => {"StrK":"StrV","ObjK":{"IntK":-1,"FloatK":1.234568},"BoolK":true,"NullK":null,"_":"ValueOnly"}

  // "-{": build a fragment (starts with +|) that can be inserted into a json
  // s|key: value is a string (can be used to escape prefix: 
  //        e.g. "s|i|..." if you want your key to start with i|)
  json(buf64, "-{", "s|i|StrK2", "StrV2", "i|IntK2", 8);
  // => +|"i|StrK2":"StrV2","IntK2":8

  // build a json on heap with a json and a fragment:
  char* buf512 = (char*)malloc(512);
  jsonHeap(buf512, 512, "o|ObjK2", buf256, buf64);
  // => {"ObjK2":{"StrK":"StrV","ObjK":{"IntK":-1,"FloatK":1.234568},"BoolK":true,"NullK":null,
  //     "_":"ValueOnly"},"i|StrK2":"StrV2","IntK2":8}
  free(buf512);

  // "s[key": a string array
  char* strArray[] = {"StrV3", "Str\"V4\""};
  // "i[key": an integer (32 bits) array
  int32_t intArray[] = {0, -2147483648, 2147483647};
  // "f#[key": a floating number array with 1 to 17 significant digits (a is 10 ... h is 17)
  double floatArray[] = {-0x1.fffffffffffffp+1023, -2.2250738585072014e-308};
  // "b[key": a boolean array
  int32_t boolArray[] = {0, 1};

  json(buf256, "s[StrArrayK", 2, strArray, "i[IntArrayK", 3, intArray,
             "fh[FloatArrayK", 2, floatArray, "b[BoolArrayK", 2, boolArray);
  // => {"StrArrayK":["StrV3","Str\"V4\""],"IntArrayK":[0,-2147483648,2147483647],
  //     "FloatArrayK":[-1.7976931348623157e+308,-2.2250738585072014e-308],"BoolArrayK":[false,true]}

  // "o[key": an array of mixed values
  // When the key of an array is empty, it is not enclosed by an object.
  char* otherArray[] = {"\"NoKeyArray\"", "[]", "{}", "null", "40", "5.55", "false"};
  json(buf128, "o[", 7, otherArray);
  // => ["NoKeyArray",[],{},null,40,5.55,false]

  // \\, \n, \b, \t, \r, \f, \" in strings will be escaped one more time
  json(buf64, "\\\n\b\t\r\f\""); // "{\"_\":\"\\\\\\n\\b\\t\\r\\f\\\"\"}"
```

#### JSON Logger
```c
#include <JsonLogger.h>

void to_console(int level, const char* json, int len) {
  char mod[LOG_MAX_LEN];
  memcpy(mod, json, len + 1);

  logModifyForHuman(level, mod);

  Serial.println(mod);
}

void to_mqtt(int level, const char* json, len) {
  if (level >= LEVEL_INFO) {
    send(json, len);
  }
}

...
  logAddSender(to_console);
  logAddSender(to_mqtt);

  logTrace("should not be logged at all if LOG_MIN_LEVEL is not changed to 0");
  logDebug("log to terminal, but not to mqtt");
  logInfo("i|status", -1, "f5|pi", 3.14159, "log to both \"terminal\" and \"mqtt\"");
  logWarn("Warning");
  logError("Error");
  logFatal("Fatal");
```
output:
```json
terminal: {1970-01-01T00:00:00Z DEBUG src/Logger.c:109 main , _ : log to terminal, but not to mqtt }

terminal: {1970-01-01T00:00:00Z INFO src/Logger.c:111 main , status :-1, pi :3.1416, _ : log to both 'terminal' and 'mqtt' }
mqtt    : {"t":"1970-01-01T00:00:00Z","i":"DEVICE UUID","l":2,"s":"src/Logger.c:111","f":"main","status":-1,"pi":3.1416,"_":"log to both \"terminal\" and \"mqtt\""}

terminal: {1970-01-01T00:00:00Z WARN src/Logger.c:113 main , _ : Warning }
mqtt    : {"t":"1970-01-01T00:00:00Z","i":"DEVICE UUID","l":3,"s":"src/Logger.c:113","f":"main","_":"Warning"}

terminal: {1970-01-01T00:00:00Z ERROR src/Logger.c:115 main , _ : Error }
mqtt    : {"t":"1970-01-01T00:00:00Z","i":"DEVICE UUID","l":4,"s":"src/Logger.c:115","f":"main","_":"Error"}

terminal: {1970-01-01T00:00:00Z FATAL src/Logger.c:117 main , _ : Fatal }
mqtt    : {"t":"1970-01-01T00:00:00Z","i":"DEVICE UUID","l":5,"s":"src/Logger.c:117","f":"main","_":"Fatal"}

terminal: {1970-01-01T00:00:00Z , l :8, src/Logger.c:119 main , _ : DATA }
mqtt    : {"t":"1970-01-01T00:00:00Z","i":"DEVICE UUID","l":8,"s":"src/Logger.c:119","f":"main","_":"DATA"}

```


### Dependencies:

Only a few C standard library functions
```c
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
```

### License:

MIT
