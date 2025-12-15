#include "HttpRequestParser.hpp"
#include <algorithm>
#include <cctype>

/**
* @brief sets the error status and logs the error message
*
* @param code the HTTP error code to set
* @param msg the error message to log
* @return bool always returns false to indicate an error occurred
*
* @note used internally to handle parsing errors and set the appropriate error status
*/
bool HttpParser::set_errstatus(int code, std::string const& msg) {
    _errStatus = code;
    std::cout << "req parsing error: " << msg << std::endl;
    return false;
}

/**
 * @brief validates the startline of a http request
 *
 * @param HttpParser _method within class HttpRequest nested in HttpParser
 * @return true or false, on false, set the _errStatus the coresponding error code
 *
 * @note example of startline: GET /index.html HTTP/1.1
 */
bool HttpParser::validateStartLine()
{
    //  GET, POST, DELETE  ---> 405: mathod not allowed
    if (_req.getMethod() != "GET" && _req.getMethod() != "POST" && _req.getMethod() != "DELETE")
        return set_errstatus(405, "Method not allowed: " + _req.getMethod());
    //  Only accept HTTP/1.1
    if (_req.getVersion() != "HTTP/1.1")
        return set_errstatus(400, "Invalid HTTP version: " + _req.getVersion());
    // Enhancement: path must start with "/"
    if (_req.getPath()[0] != '/')
        return set_errstatus(400, "Path must start with '/': " + _req.getPath());
    // Enhancement: path cannot be too long check
    if (_req.getPath().size() > 2048)
        return set_errstatus(414, "Request_URI too long: " + _req.getPath());
    // Enhancement: path cannot have empty space or controling chars
    for (size_t i = 0; i < _req.getPath().size(); ++i){
        char c = _req.getPath()[i];
        if (c < 31 || c == ' ')
            return set_errstatus(400, "Path must not have empty space or controling chars: " + _req.getPath());
    }
    return true;
}

/**
 * @brief validates the headers of a http request
 *
 * @param HttpParser map headers within class HttpRequest nested in HttpParser
 * @return true or false, on false, set the _errStatus the coresponding error code
 *
 * @note exxample of headers:
 * POST /login HTTP/1.1
 * Host: example.com
 * User-Agent: curl/7.81.0
 * Content-Type: application/x-www-form-urlencoded
 * Content-Length: 27
 */
bool    HttpParser::validateHeaders()
{
    bool hasHost = false;
    std::set<std::string> seenKeys;
    size_t totalSize = 0;

    for (std::map<std::string, std::string>::const_iterator it = _req.getHeaders().begin(); it != _req.getHeaders().end(); ++it)
    {
        const std::string& key = it->first;
        const std::string& value = it->second;
        totalSize += key.size() + value.size();

        // Enhancement: the headers can not be too big, like todal length 8192 8KB
        if (totalSize > 8192)
            return set_errstatus(431, "Total header size too big");
        // a single header can not be too long: 431, Request Header Fields Too Large
        if (key.size() > 1024 || value.size() > 4096)
            return set_errstatus(431, "Single header field too large: " + key);
        // validator key: key cannot has space in it
        for (size_t i = 0; i < key.size(); ++i) {
            if (!isgraph(key[i]) || key[i] == ':') 
                return set_errstatus(400, "Invalid header key: " + key);
        }
        //validateMandatoryHeaders: loop though all keys, has to find host
        if (key == "Host"){
            if (hasHost)
                return set_errstatus(400, "Multiple Host headers found.");
            hasHost = true;
        }
        // validateRepeatingHeaders: loop though all keys, can not have repeating keys, multiple Hosts
        if (seenKeys.count(key)) 
            return set_errstatus(400, "Duplicate header key found: " + key); 
        seenKeys.insert(key);
        // Enhancement(optional, did not do):  reject control characters (CR/LF, other CTLs, DEL). Also reject embedded CR/LF (obs-fold is obsolete) and enforce trimming/size limits
        // validateContentLength: can not be too long like minus number ---> four hundred  or too big(Payload Too Large) ---> 43113
        if (key == "Content-Length"){
            // can not be empty
            if (value.empty())
                return set_errstatus(400, "Empty Content-Length value.");
            // can not start with zero
            if (value.size() > 1 && value[0] == '0')
                return set_errstatus(400, "Content-Length cannot start with zero.");
            // can not have non digits
            for (size_t i = 0; i < value.length(); ++i){
                if (!isdigit(value[i]))
                    return set_errstatus(400, "Non-digit character in Content-Length value.");
            }
            // can not be minus
            long long len = atoll(value.c_str());
            if (len < 0)
                return set_errstatus(400, "Negative Content-Length value.");
             // can not be too big, 100 MB, max, ask lin or lucio what should be the max
            if (len > 1024 * 1024 * 100)
                return set_errstatus(413, "Content-Length too large.");
            _bodyLength = static_cast<size_t>(len);
        }
    }
    // it has to have ont and only one host    // headers["Content-Type"] = "text/html";
    // headers["Content-Length"] = std::to_string(body.size());
    if (!hasHost)
        return set_errstatus(400, "Missing Host header.");
    return true;
}

/**
 * @brief validates the body of a http request
 *
 * @param HttpParser _body within class HttpRequest nested in HttpParser
 * @return true or false, on false, set the _errStatus the coresponding error code
 *
 * @example example of body: username=John&password=1234
 * @note Post must have body, body must fullfill ContentLength, for GET / DELETE, it is not an error to have body
 * @note
 * - body string length has to be as long as the sontent-Length says
 * - here, i can not have this check because the body might be not fully received yet
 * - incomplete body is not an error during streaming)
 */
bool HttpParser::validateBody(){
    // Enhancement: for textual content types, body cannot contain null byte
    // Binary content (images, multipart, application/octet-stream, etc.) may contain NULs and must be allowed.
    const std::map<std::string, std::string>& headers = _req.getHeaders();
    std::string contentType;
    std::map<std::string, std::string>::const_iterator it = headers.find("Content-Type");
    if (it != headers.end()) {
        contentType = it->second;
        //Content-Type: text/html; charset=UTF-8
        size_t semi = contentType.find(';');
        if (semi != std::string::npos)
            contentType = contentType.substr(0, semi);
        // eg contentType = "Text/HTML; Charset=UTF-8"; ->  contentType = "text/html; charset=utf-8";
        for (size_t i = 0; i < contentType.size(); ++i)
            contentType[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(contentType[i])));
    }
    bool isTextual = false;
    if (contentType.empty() ||
        contentType.rfind("text/", 0) == 0 ||
        contentType == "application/x-www-form-urlencoded" ||
        contentType == "application/json" ||
        contentType == "application/xml")
        isTextual = true;
    if (isTextual) {
        if (_req.getBody().find('\0') != std::string::npos) 
            return set_errstatus(400, "Body contains null byte for textual Content-Type: " + contentType);
    }

    if (_req.getMethod() == "POST")
    {
        // For POST: require Content-Length only if there is a non-empty body. 
        bool hasContentLength = _req.getHeaders().count("Content-Length") != 0;
        if (!hasContentLength) {
            // No Content-Length header: allowed only for an empty body.
            if (!_req.getBody().empty())
                return set_errstatus(400, "POST request has body but missing Content-Length header.");
        }
        else {
            // Content-Length present: ensure body size does not exceed declared length.
            if (_req.getBody().size() > _bodyLength)
                return set_errstatus(400, "POST request body length exceeds Content-Length.");
        }
    }
    return true;
}

/**
 * @brief parse the startline of an HTTP request
 *
 * @param str startline
 * @return void
 *
 * @note iss : in stringstream
 */
void HttpParser::parseStartLine(const std::string& startline){
    std::istringstream ss(startline);
    std::vector<std::string> fields;
    std::string tmp;

    while (ss >> tmp)
        fields.push_back(tmp);
    //there is actullally some pre_validatingg here
    if (fields.size() != 3){
        _errStatus = 400;
        _state = ERROR;
        return ;
    }
    _req.setMethod(fields[0]);
    _req.setPath(fields[1]);
    _req.setVersion(fields[2]);
    _state = HEADERS;
}

/**
 * @brief parse one header of an HTTP request
 *
 * @param string headerline
 * @return void
 *
 * @note first check if headers are done, if yes, the line is '\t\n', then change state to body or done
 */
void HttpParser::parseHeaderLine(const std::string& headerline){
    if (headerline.empty())
    {
        // try to find content length in map to see it there is body
        const std::map<std::string, std::string>& headers = _req.getHeaders();
        std::map<std::string, std::string>::const_iterator it = headers.find("content-length");
        if (it != headers.end()){
            _bodyLength = std::stoi(it->second);
            _state = BODY;
        }
        else
            _state = DONE;
        return;
    }
    size_t dd = headerline.find(":");
    if (dd == std::string::npos)
        return;
    std::string key = headerline.substr(0, dd);
    std::string val = headerline.substr(dd + 1);
    key = httpUtils::trim_space(key);
    val = httpUtils::trim_space(val);
    _req.addHeader(key, val);
}

/**
 * @brief parse body of an HTTP request
 *
 * @param str headerline
 * @return void
 *
 * @note iss : in stringstream
 */
void HttpParser::parseBody(size_t pos)
{
    size_t              available = _buffer.size() - pos;
    const std::string&  curBody = _req.getBody();
    size_t              toRead = std::min(available, _bodyLength - curBody.size());
    std::string         newBody = _req.getBody();       // copy (const ok)

    newBody += _buffer.substr(pos, toRead);             // modify copy
    _req.setBody(newBody);
    pos += toRead;
    if (_req.getBody().size() >= _bodyLength)
        _state = DONE;
}

/**
 * @brief parse an HTTP request, including startline, headers and body
 *
 * @param str rawLine (the data rawLine might be ramdom pieces, not neccessarily a full line)
 * @return an HttpRequest object
 *
 * @note this one is going to called mamy times, basically whenever recv() some new bytes,
 */
HttpRequest HttpParser::parseHttpRequest(const std::string& rawLine)
{
    _buffer += rawLine;

    size_t pos = 0;
    // first startine and headers and body
    while (_state != DONE)
    {
        if (_state < BODY)
        {
            size_t end = _buffer.find("\r\n", pos);
            if (end == std::string::npos)
                return HttpRequest();
            std::string line = _buffer.substr(pos, end - pos);
            pos = end + 2;
            if (!line.empty() && line.back() == '\r')  // check is the last one is \r
                line.pop_back();    //remove the last char \r
            if (_state == START_LINE){
                parseStartLine(line);
                if (_state == ERROR)
                    return HttpRequest();
            }
            if (_state == HEADERS)
                parseHeaderLine(line);
        }
        if (_state == BODY){
            parseBody(pos);
            break;
        }
    }
    //move post
    if (pos > 0)
        _buffer.erase(0, pos);
    if (_state == DONE) {
        if (!validateStartLine() || !validateHeaders() || !validateBody()) {
            _state = ERROR;
            return HttpRequest();
        }
        return _req;
    }
    return HttpRequest();
}
