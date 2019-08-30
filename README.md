JSON Logger
======
An easy-to-use, small, fast, and portable JSON builder and logger for IoT firmware data gathering.

### Usage:

#### JSON Builder
```c
#include <JsonLogger.h>
...
  // To build a json one level at a time, call json() and supply a char[] & key-value pairs as arguments:

  char buf3[3], buf256[256], buf64[64], buf512[512];
  // key: value is a string
  // s|key: value is a string (can be used to escape prefix: e.g. "s|i|..." if you want your key to start with i|)
  // i|key: value is an integer (32 bits)
  // d|*key: value is a floating number with 1 to 17 significant digits; argument should be a double
  // b|key: value is a boolean
  // o|key: value is anything else (object, array, null)
  json(buf256, "str_key1", "str1", "i|int_key1", 7, "d|3double_key1", 3.14159,
             "b|boolean_key1", 1, "o|object_key1", "{}", "o|array_key1", "[]", "o|null_key1", "null");
  // => {"str_key1":"str1","int_key1":7,"double_key1":3.14,"boolean_key1":true,"object_key1":{},"array_key1":[],"null_key1":null}

  // if an argument has no matching pair, is not a fragment or part of an array, it will be a value to empty key
  json(buf512, "value only");
  // => {"":"value only"}

  // "-{": build a fragment (starts with +|) that can be inserted into a json
  json(buf64, "-{", "str_key2", "str2", "i|int_key2", 8);
  // => +|"str_key2":"str2","int_key2":8

  // build a json with an object json and a fragment
  json(buf512, "o|obj", buf256, buf64);
  // => {"obj":{"str_key1":"str1","int_key1":7,"double_key1":3.14,"boolean_key1":true,"object_key1":{},"array_key1":[],"null_key1":null},"str_key2":"str2","int_key2":8}

  // "s[": a string array, the next argument is the number of items
  json(buf64, "s[", 2, "str3", "str4\"inquote\"");
  // => ["str3","str4\"inquote\""]

  // "i[": an integer (32 bits) array
  json(buf64, "i[", 3, 0, -2147483648, 2147483647);
  // => [0,-2147483648,2147483647]

  // "d[*": a floating number array with 1 to 17 significant digits (a is 10, b is 11 ... h is 17); arguments should be doubles
  json(buf64, "d[1", 4, 0.0, 0.01, 4.44, 1.2345678901234567890);
  // => [0,0.01,4,1]
  json(buf64, "d[7", 4, 0.0, 0.01, 4.44, 1.2345678901234567890);
  // => [0,0.01,4.44,1.234568]
  json(buf64, "d[h", 4, 0.0, 0.01, 4.44, -1.2345678901234567890);
  // => [0,0.01,4.4400000000000004,-1.2345678901234567]

  // "b[": a boolean array
  json(buf64, "b[", 2, 0, 1); 
  // => [false,true]

  // "o[": an array of mixed values
  json(buf64, "o[", 7, "[]", "{}", "null", "40", "5.55", "false", "\"str5\""); 
  // => [[],{},null,40,5.55,false,"str5"]
```

#### JSON Logger
```c
#include <JsonLogger.h>

void to_console(int level, const char* json) {
  if (level >= LEVEL_INFO) {
    printf("%s\n", json);
  }
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
  logInfo("i|status", -1, "d|5pi", 3.14159, "Info");
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
