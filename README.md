C JSON Builder
======
An easy-to-use, small, fast and portable JSON builder for firmware logging and communication. No more ugly and error-prone escaping quotes.

### Usage:

```c
#include <json_builder.h>
...
  // To build a json one level at a time, call json() and supply a char[] & key-value pairs as arguments:

  char buf3[3], buf256[256], buf64[64], buf512[512];
  // key: value is a string
  // i|key: value is an integer
  // d|*key: value is a floating number with 1 to 17 significant digits; input should be a double
  // b|key: value is a boolean
  // o|key: value is anything else (object, array, null)
  json(buf256, "str_key1", "str1", "i|int_key1", 7, "d|3double_key1", 3.14159,
             "b|boolean_key1", 1, "o|object_key1", "{}", "o|array_key1", "[]", "o|null_key1", "null");
  // => {"str_key1":"str1","int_key1":7,"double_key1":3.14,"boolean_key1":true,"object_key1":{},"array_key1":[],"null_key1":null}

  // "-{": build a fragment (starts with +|) that can be inserted into a json
  json(buf64, "-{", "str_key2", "str2", "i|int_key2", 8);
  // => +|"str_key2":"str2","int_key2":8

  // add an object and a fragment to a json
  json(buf512, "o|obj", buf256, buf64);
  // => {"obj":{"str_key1":"str1","int_key1":7,"double_key1":3.14,"boolean_key1":true,"object_key1":{},"array_key1":[],"null_key1":null},"str_key2":"str2","int_key2":8}

  // if an argument has no matching pair, is not a fragment or part of an array, it will be a value to empty key
  json(buf512, "value only");
  // => {"":"value only"}

  // "s[": a string array, the argument after is the number of items
  json(buf64, "s[", 2, "str3", "str4\"inquote\"");
  // => ["str3","str4\"inquote\""]

  // "i[": an integer array
  json(json_c, "i[", 3, 0, 10, 20); 
  // => [0,10,20]

  // "d[*": a floating number array with 1 to 17 significant digits (a is 10, b is 11 ... h is 17); inputs should be doubles
  json(buf64, "d[1", 4, 0.0, 0.01, 4.44, 1.2345678901234567890);
  // => [0,0.01,4,1]
  json(buf64, "d[7", 4, 0.0, 0.01, 4.44, 1.2345678901234567890);
  // => [0,0.01,4.44,1.234568]

  // "b[": a boolean array
  json(buf64, "b[", 2, 0, 1); 
  // => false,true]

  // "o[": an array of mixed values
  json(buf64, "o[", 7, "[]", "{}", "null", "40", "5.55", "false", "\"str5\""); 
  // => [[],{},null,40,5.55,false,"str5"]  
```

### Dependencies:

Only the basic ANSI C stuff
```c
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
```

### License:

MIT
