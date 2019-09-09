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
  // If the last parameter has no matching pair, it will be a value to "_" key
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

void to_console(int level, const char* json) {
  char mod[LOG_MAX_LEN];
  strcpy(mod, json);

  logModifyForHuman(level, mod);

  Serial.println(mod);
}

void to_mqtt(int level, const char* json) {
  if (level >= LEVEL_WARN) {
    send(json);
  }
}

...
  logAddSender(to_console);
  logAddSender(to_mqtt);

  logTrace("should not be logged at all if LOG_MIN_LEVEL is not changed to 0");
  logDebug("Debug");
  logInfo("i|status", -1, "f5|pi", 3.14159, "Info");
  logWarn("Warning");
  logError("Error");
  logFatal("Fatal");

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
