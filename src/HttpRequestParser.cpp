#include "HttpRequestParser.hpp"

// example http request with a content lenth
// POST /login HTTP/1.1
// Host: example.com
// User-Agent: curl/7.81.0 
// Content-Type: application/x-www-form-urlencoded 
// Content-Length: 27 
// \r\n
// username=John&password=1234

// a helper to trim empty space
static std::string trim_space(std::string str)
{
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return str.substr(start, end - start + 1);
}

bool HttpParser::validateStartLine()
{
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
    //something missing
    if (_req.getMethod().empty() || _req.getPath().empty() || _req.getVersion().empty())
    {
        _errStatus = 400;
        return false;
    }
    return true;
}

bool    HttpParser::validateHeaders()
{
    bool hasHost = false;

    for (std::map<std::string, std::string>::const_iterator it = _req.getHeaders().begin(); it != _req.getHeaders().end(); ++it)
    {
        const std::string& key = it->first;
        const std::string& value = it->second;

        if (key == "Host")
        {
            //validateMandatoryHeaders: loop though all keys, has to find host
            if (hasHost)   // 多个 Host
            {
                _errStatus = 400;  // Bad Request
                return false;
            }
            hasHost = true;
        }

        //validateRepeatingHeaders: loop though all keys, can not have repeating keys, multiple Hosts
        
        //validateContentLength: can not be too long like minus number ---> 300 or too big(Payload Too Large) ---> 413
        if (key == "Content-Length")
        {
            for (size_t i = 0; i < value.length(); ++i) // all digits
            {
                if (!isdigit(value[i]))
                {
                    _errStatus = 400;
                    return false;
                }
            }
            long long len = atoll(value.c_str()); 
            if (len < 0)                                //positive
            {
                _errStatus = 400; // negative length
                return false;
            }
            if (len > 1024 * 1024 * 100)  // 100 MB, max, ask lin or lucio what should be the max
            {
                _errStatus = 413; // Payload Too Large
                return false;
            }
            _bodyLength = static_cast<size_t>(len);
        }
    }
    if (!hasHost) {
        _errStatus = 400;
        return false;
    }

    return true;
}

 // Post must have body, body must fullfill ContentLength, for GET / DELETE, it is not an error to have body
bool HttpParser::validateBody(){
    if (_req.getMethod() == "POST")
    {
        if (_bodyLength == 0 && !_req.getHeaders().count("Content-Length"))
        {
            _errStatus = 400;
            return false;
        }
        if (_req.getBody().size() < _bodyLength)
        {
            _errStatus = 400;
            return false;
        }
        if (_req.getBody().size() > _bodyLength) // in theory, it should not happen
        {
            _errStatus = 400;
            return false;
        }
    }
    return true;
}

//below one is private, tool to be called in parseHttpRequest.
// <start-line>\r\n: Method sp _path sp version crlf(Carriage Return and Line Feed)
// Get /index.html HTTP/1.1\r\n
void HttpParser::parseStartLine(const std::string& startline)
{
    std::istringstream ss(startline);   //ss : stringstream
    std::string method, path, version;
    ss >> method >> path >> version;
    _req.setMethod(method);
    _req.setPath(path);
    _req.setVersion(version);
    _state = HEADERS;
}

//below one is private, it will be recalled for a few times.
void HttpParser::parseHeaderLine(const std::string& headerline){
    // first check if headers are done, \r\n will be removed \r\n
    if (headerline.empty())
    {
        // try to find content length in map to see it there is bodu
        const std::map<std::string, std::string>& headers = _req.getHeaders();
        std::map<std::string, std::string>::const_iterator it = headers.find("Content-Length");
        // if the abouve find it, it returns the content length pair
        // if not find, return map.end. end of one step past the last element
        //so this is find it 
        if (it != headers.end()){
            _bodyLength = std::stoi(it->second);
            _state = BODY;
        }
        else
            _state = DONE;
        return;
    }
    //if not: still in the headers stage: find ":"
    size_t dd = headerline.find(":");
    if (dd == std::string::npos)
        return;
    std::string key = headerline.substr(0, dd);
    std::string value = headerline.substr(dd + 1);
    key = trim_space(key);
    value = trim_space(value);
    _req.addHeader(key, value); 
}

void HttpParser::parseBody(size_t pos)
{   
    size_t available = _buffer.size() - pos;
    const std::string& curBody = _req.getBody();
    size_t toRead = std::min(available, _bodyLength - curBody.size());
    _req.getBody() += _buffer.substr(pos, toRead);
    pos += toRead;
    if (_req.getBody().size() >= _bodyLength)
        _state = DONE;
}

// this one is going to called mamy times, basically whenever recv() some new bytes, 
// the data might be ramdom pieces, not neccessarily a full line
HttpRequest HttpParser::parseHttpRequest(const std::string& rawLine)
{
    _buffer += rawLine;
    
    size_t pos = 0;
    // first startine and headers and body 
    while (_state != DONE) // Lucio addition, maybe add || _state != ERROR
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
    if (!validateStartLine() || !validateHeaders() || !validateBody()){ 
        _state = ERROR;
        return HttpRequest();
    }
    if (_state == DONE)
        return _req;
    return HttpRequest();
}

int HttpParser::getState(){
    return _state;
}

int HttpParser::getErrStatus(){
    return _errStatus;
}

/* Lucio Suggestion, maybe generate an error response here that will return a page according to the error code
Currently HttpResponseHandler just deals with requests that have GET, POST, DELETE. What happen if the request never
has any of them? So far it throws and kills the program, in error it should just return one of the error pages.

Could be added a generateErrorRequestResponse()
*/

