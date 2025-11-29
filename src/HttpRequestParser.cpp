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
    if (_method != "GET" && _method != "POST" && _method != "DELETE") //  GET, POST, DELETE  ---> 405
    {
        _errStatus = 405;
        return false;
    }
    if (_version != "HTTP/1.1") //  Only accept HTTP/1.1
    {
        _errStatus = 400;
        return false;
    }
    return true;
}

bool    HttpParser::validateHeaders()
{
    //validateMandatoryHeaders: loop though all keys, has to find host

    //validateRepeatingHeaders: loop though all keys, can not have repeating keys, multiple Hosts

    //validateContentLength: can not be too long like minus number ---> 300 or too big(Payload Too Large) ---> 413

    return true;
}

 // Post must have body, body must fullfill ContentLength
bool HttpParser::validateBody(){
    return true;
}

//below one is private, tool to be called in parseHttpRequest.
void HttpParser::parseStartLine(const std::string& startline)
{
    std::istringstream ss(startline);   //ss : stringstream
    ss >> _method >> _path >> _version;

    if (_method.empty() || _path.empty() || _version.empty())
        throw std::runtime_error("Something missing in http request starting line");
    _state = HEADERS;
}

//below one is private, it will be recalled for a few times.
void HttpParser::parseHeaderLine(const std::string& headerline){
    // first check if headers are done, \r\n will be removed \r\n
    if (headerline.empty())
    {
        std::map<std::string, std::string>::iterator it = _requestHeaders.find("Content-Length"); // try to find content length in map to see it there is bodu
        // if the abouve find it, it returns the content length pair
        // if not find, return map.end. end of one step past the last element
        //so this is find it 
        if (it != _requestHeaders.end()){
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
    _requestHeaders[key] = value;
}

void HttpParser::parseBody(size_t pos)
{   
    size_t available = _buffer.size() - pos;
    size_t toRead = std::min(available, _bodyLength - _body.size());
    _body += _buffer.substr(pos, toRead);
    pos += toRead;
    if (_body.size() >= _bodyLength)
        _state = DONE;
}

// this one is going to called mamy times, basically whenever recv() some new bytes, 
// the data might be ramdom pieces, not neccessarily a full line
HttpRequest HttpParser::parseHttpRequest(const std::string& rawLine)
{
    _buffer += rawLine;
    
    size_t pos = 0;
    while (_state != DONE  ) // Lucio addition, maybe add || _state != ERROR
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
            if (!validateStartLine())
                // _state = ERROR
                throw std::runtime_error("405 Method Not Allowed");
            if (_state == HEADERS)
            {
                parseHeaderLine(line);
                if (_state > HEADERS)
                {
                    if (!validateHeaders())
                        // _state = ERROR
                        throw std::runtime_error("400 Bad Request: invalid headers");
                    break;
                }
            }
        }
        if (_state == BODY)
        {
            parseBody(pos);
            if (!validateBody())
                // _state = ERROR
                throw std::runtime_error("400 Bad Request: something wrong with body");
            break;
        }
    }
    if (pos > 0)
        _buffer.erase(0, pos);
    if (_state == DONE)
        return HttpRequest(_method, _path, _version, _body, _requestHeaders);
//  if (_state == ERROR)
//      return 
    return HttpRequest();
}