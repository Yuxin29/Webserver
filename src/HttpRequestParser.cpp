#include "HttpRequestParser.hpp"

/**
 * @brief Trims the empty space \t at the beginning and the end of a string
 *
 * @param str a string with possible '\t' at the beginning and end
 * @return a string without any '\t' at the beginning or end
 *
 * @note Currently supports 400, 405, 413, 431. Default is 400.
 */
static std::string trim_space(std::string str)
{
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

/**
 * @brief Builds an HTTP error response for a given status code.
 *
 * @param status HTTP status code (e.g., 400, 405, 413, 431)
 * @return HttpResponse object representing the error page
 *
 * @note Currently supports 400, 405, 413, 414(addable, not mandatory), 431. Default is 400.
 */
HttpResponse buildErrorResponse(int status)
{
    std::string reason;
    std::string body;

    switch (status)
    {
        case 400:
            reason = "Bad Request";
            body = "<h1>400 Bad Request</h1>";
            break;
        case 405:
            reason = "Method Not Allowed";
            body = "<h1>405 Method Not Allowed</h1>";
            break;
        case 413:
            reason = "Payload Too Large";
            body = "<h1>413 Payload Too Large</h1>";
            break;
        case 414:
            reason = "URI Too Long";
            body = "<h1>414 URI Too Long</h1>";
            break;
        case 431:
            reason = "Request Header Fields Too Large";
            body = "<h1>431 Request Header Fields Too Large</h1>";
            break;
        default:
            reason = "Bad Request";
            body = "<h1>400 Bad Request</h1>";
            break;
    }
    std::map<std::string, std::string> headers;
    headers["Content-Type"] = "text/html";
    headers["Content-Length"] = std::to_string(body.size());
    return HttpResponse("HTTP/1.1", status, reason, body, headers, false, false);
}

/**
 * @brief validates the startline of a http request
 *
 * @param HttpParser _method within class HttpRequest nested in HttpParser
 * @return true or false, on false, set the _errStatus the coresponding error code
 *
 * @note exxample of startline: GET /index.html HTTP/1.1
 */
bool HttpParser::validateStartLine()
{
    //something missing, this one check first to avoid seg fault
    if (_req.getMethod().empty() || _req.getPath().empty() || _req.getVersion().empty()){
        _errStatus = 400;
        return false;
    }
    //  GET, POST, DELETE  ---> 405: mathod not allowed
    if (_req.getMethod() != "GET" && _req.getMethod() != "POST" && _req.getMethod() != "DELETE"){
        _errStatus = 405;
        return false;
    }
    //  Only accept HTTP/1.1
    if (_req.getVersion() != "HTTP/1.1"){
        _errStatus = 400;
        return false;
    }
    //path cannot have empty space or path cannot be too long check, i think it is not necessary
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

    for (std::map<std::string, std::string>::const_iterator it = _req.getHeaders().begin(); it != _req.getHeaders().end(); ++it)
    {
        const std::string& key = it->first;
        const std::string& value = it->second;
        std::set<std::string> seenKeys;

        // validator key: key cannot has space in it
        for (size_t i = 0; i < key.size(); ++i) {
            if (!isgraph(key[i]) || key[i] == ':') {
                _errStatus = 400;
                return false;
            }
        }
        //validateMandatoryHeaders: loop though all keys, has to find host
        if (key == "Host"){
            if (hasHost){
                _errStatus = 400; 
                return false;
            }
            hasHost = true;
        }
        // validateRepeatingHeaders: loop though all keys, can not have repeating keys, multiple Hosts
        if (seenKeys.count(key)) {
            _errStatus = 400;
            return false;
        }
        seenKeys.insert(key);
        //validateContentLength: can not be too long like minus number ---> four hundred  or too big(Payload Too Large) ---> 43113
        if (key == "Content-Length"){
            // can not be empty
            if (value.empty()){
                _errStatus = 400;
                return false;
            }
            // can not start with zero
            if (value.size() > 1 && value[0] == '0'){
                _errStatus = 400;
                return false;
            }
            // can not have non digits
            for (size_t i = 0; i < value.length(); ++i){
                if (!isdigit(value[i])){
                    _errStatus = 400;
                    return false;
                }
            }
            // can not be minus
            long long len = atoll(value.c_str()); 
            if (len < 0){
                _errStatus = 400;
                return false;
            }
             // can not be too big, 100 MB, max, ask lin or lucio what should be the max
            if (len > 1024 * 1024 * 100){
                _errStatus = 413;
                return false;
            }
            _bodyLength = static_cast<size_t>(len);
        }
        // header can not be too long:   431, Request Header Fields Too Large
        if (key.size() > 1024 || value.size() > 4096){
            _errStatus = 431; 
            return false;
        }
    }
    // it has to have ont and only one host
    if (!hasHost){
        _errStatus = 400;
        return false;
    }
    return true;
}

/**
 * @brief validates the body of a http request
 *
 * @param HttpParser _body within class HttpRequest nested in HttpParser
 * @return true or false, on false, set the _errStatus the coresponding error code
 *
 * @note exxample of startline: username=John&password=1234
 * @note Post must have body, body must fullfill ContentLength, for GET / DELETE, it is not an error to have body
 */
bool HttpParser::validateBody(){
    if (_req.getMethod() == "POST")
    {
        // POST has to have body string and in headers, it has to have content-Length"
        if (_bodyLength == 0 && !_req.getHeaders().count("Content-Length")){
            _errStatus = 400;
            return false;
        }
        // body string length has to be as long as the sontent-Length says
        if (_req.getBody().size() < _bodyLength){
            _errStatus = 400;
            return false;
        }
        // body lenth can not be too long: in theory it should not happen
        if (_req.getBody().size() > _bodyLength){
            _errStatus = 400;
            return false;
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
    std::string method, path, version;

    ss >> method >> path >> version;
    _req.setMethod(method);
    _req.setPath(path);
    _req.setVersion(version);
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
        std::map<std::string, std::string>::const_iterator it = headers.find("Content-Length");
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
    std::string value = headerline.substr(dd + 1);
    key = trim_space(key);
    value = trim_space(value);
    _req.addHeader(key, value); 
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
        if (_state == START_LINE || _state == HEADERS)
        {
            size_t end = _buffer.find("\r\n", pos);
            if (end == std::string::npos)
                return HttpRequest();
            std::string line = _buffer.substr(pos, end - pos);
            pos = end + 2;
            if (!line.empty() && line.back() == '\r')  // check is the last one is \r
                line.pop_back();    //remove the last char \r
            if (_state == START_LINE)
                parseStartLine(line);
            if (_state == HEADERS){
                parseHeaderLine(line);
                if (_state > HEADERS)
                    break;
            }
        }
        if (_state == BODY){
            parseBody(pos);
            break;
        }
    }
    //move post
    if (pos > 0)
        _buffer.erase(0, pos);
    //at the end, validate 
    if (!validateStartLine() || !validateHeaders() || !validateBody())
        _state = ERROR;
    // not finished or error
    if (_state != DONE || _state == ERROR)
        return HttpRequest();
    return _req;
}

int HttpParser::getState(){
    return _state;
}

int HttpParser::getErrStatus(){
    return _errStatus;
}