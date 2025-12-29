/*!
 * @ testing.md
 * @brief Core C++ and HTTP concepts used in building an HTTP server.
 *
 * This document includes:
 * - testing with the tester
 * - testing with curl -i
 *
 * @details
 * This file uses Doxygen HTML-style block comments
 * Created by Yuxin for practicing documentation
 */

# 1. testing with the tester
  this tester comes from 42.

## Usages
    - in one terminal: ./webserv configuration/tester.conf
    - in another terminal: ./tester http://localhost:8080

# 2. testing with curl -i
  this print out the returning infor

## Usages
    Test GET http://localhost:8080/directory  -> FATAL ERROR ON LAST TEST: bad status code
    -> curl -i http://localhost:8080/directory

