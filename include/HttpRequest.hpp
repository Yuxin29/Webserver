#pragma once

#include <string>
#include <map>

// <start-line>\r\n
// <header1: value>\r\n
// <header2: value>\r\n
// ...\r\n
// \r\n
// <body>
class HttpRequest{
private:
    std::string                         _method;
    std::string                         _requestPath;
    std::string                         _version;
    std::map<std::string, std::string>  _requestHeaders;
    std::string                         _body;

public:
    //Orthodox not all needed: defined just here
    HttpRequest() {}

    //getters
    const std::string&                         getMethod() const;
    const std::string&                         getPath() const;
    const std::string&                         getVersion() const;
    const std::map<std::string, std::string>&  getHeaders() const;
    const std::string&                         getBody() const;

    //setters
    void setMethod(const std::string &m);
    void setPath(const std::string &m);
    void setVersion(const std::string &m);
    void addHeader(const std::string& k, const std::string& v);
    void setBody(const std::string &m);
};

// method required by this subject: GET, POST, DELETE
// method required by this subject: GET, POST, DELETE