/*!
 * @file http_concepts.md
 * @brief Core C++ and HTTP concepts used in building an HTTP server.
 *
 * This document includes:
 * - std::map usage
 * - std::istringstream
 * - iterator it
 * - `stat`
 *
 * @details
 * This file uses Doxygen HTML-style block comments
 * Created by Yuxin for practicing documentation
 */

# 1. `std::map` — Ordered Key–Value Storage

`std::map` is widely used to store HTTP headers.

## Properties

- Keys are **unique**
- Keys remain **sorted**
- Operations (lookup/insertion/deletion) are **O(log n)**
- Implemented using a **Red–Black tree**

## Example
    MAP container that stores key–value pairs

```cpp
#include <map>
#include <string>
std::map<std::string, std::string> headers;
headers["Host"] = "localhost:8080";
headers["User-Agent"] = "curl/7.68.0";
headers["Accept"] = "*/*";
headers["Content-Length"] = "12";
```

# 2. `std::istringstream` — maybe a bit like a automatic getnext line

`std::map` is a class that string flows as a stream (eg. reading from a file or socket)

## Example
    MAP container that stores key–value pairs

```cpp
#include <map>
#include <string>
std::istringstream ss("hello world 42");
std::string a, b;
int num;
ss >> a >> b >> num;
// a = "hello"
// b = "world"
// num = 42
```

# 3. Iterators — Accessing Elements in  `std::map` or vector
    a `std::map` stores key–value pairs internally as something similar to: `pair<const Key, Value>` 

## Example 1
    Declaring and Using a Map Iterator

```cpp
std::map<std::string, std::string> headers;
std::map<std::string, std::string>::iterator it;

for (it = headers.begin(); it != headers.end(); ++it){
    std::cout << "Key: "   << it->first << " | Value: " << it->second << std::endl;
}
```

## Example 2
    Declaring and Using a Vector Iterator

```cpp
std::map<std::string, std::string> headers;
std::map<std::string, std::string>::iterator it;

for (it = headers.begin(); it != headers.end(); ++it) {
    std::cout << "Key: "   << it->first 
              << " | Value: " << it->second 
              << std::endl;
}
```

# 4. Retrieve File Information (`stat`)

## Function Prototype

```c
#include <sys/stat.h>
int stat(const char *pathname, struct stat *statbuf);
```

## Return Values
0 → Success; statbuf is filled with file information
-1 → Error; errno is set (e.g., file does not exist)

## Struct 
```c
struct stat {
    st_mode;        //file type & permissions.
    st_size;        //file size.
    st_mmine;       //ast modification time, etc.
}
```