#include "HttpRequestParser.hpp"

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
    if (_req.getMethod() != "GET" && _req.getMethod() != "POST" && _req.getMethod() != "DELETE"){
        _errStatus = 405;
        std::cout << "Method not allowed: " << _req.getMethod() << std::endl;
        return false;
    }

    if (_req.getVersion() != "HTTP/1.1"){
        _errStatus = 400;
        std::cout << "Invalid HTTP version: " << _req.getVersion() << std::endl;
        return false;
    }

    if (_req.getPath()[0] != '/'){
        _errStatus = 400;
        std::cout << "Path must not start with '/': " << _req.getPath() << std::endl;
        return false;
    }

    if (_req.getPath().size() > 2048){
        _errStatus = 400;
        std::cout << "Request_URI too long" << _req.getPath() << std::endl;
        return false;
    }

    for (size_t i = 0; i < _req.getPath().size(); ++i){
        char c = _req.getPath()[i];
        if (c < 31 || c == ' '){
            _errStatus = 400;
            std::cout << "Path must not have empty space or controling chars: " << _req.getPath() << std::endl;
            return false;
        }
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

        if (totalSize > 8192){
            _errStatus = 431;
            std::cout << "Total header size to big" << std::endl;
            return false;
        }

        if (key.size() > 1024 || value.size() > 4096){
            _errStatus = 431;
            std::cout << "Header field too large: " << key << std::endl;
            return false;
        }

        for (size_t i = 0; i < key.size(); ++i) {
            if (!isgraph(key[i]) || key[i] == ':') {
                _errStatus = 400;
                std::cout << "Invalid header key: " << key << std::endl;
                return false;
            }
        }

        if (key == "host"){
            if (hasHost){
                _errStatus = 400;
                std::cout << "Multiple Host headers found." << std::endl;
                return false;
            }
            hasHost = true;
        }

        if (seenKeys.count(key)) {
            _errStatus = 400;
            std::cout << "Duplicate header key found: " << key << std::endl;
            return false;
        }

        seenKeys.insert(key);
        if (key == "content-length"){

            if (value.empty()){
                _errStatus = 400;
                std::cout << "Empty Content-Length value." << std::endl;
                return false;
            }

            if (value.size() > 1 && value[0] == '0'){
                _errStatus = 400;
                std::cout << "Content-Length cannot start with zero." << std::endl;
                return false;
            }

            for (size_t i = 0; i < value.length(); ++i){
                if (!isdigit(value[i])){
                    _errStatus = 400;
                    std::cout << "Non-digit character in Content-Length value." << std::endl;
                    return false;
                }
            }

            long long len = atoll(value.c_str());
            if (len < 0){
                _errStatus = 400;
                std::cout << "Negative Content-Length value." << std::endl;
                return false;
            }

            if (len > 1024 * 1024 * 100){
                _errStatus = 413;
                std::cout << "Content-Length too large." << std::endl;
                return false;
            }
            _bodyLength = static_cast<size_t>(len);
        }
    }

    if (!hasHost){
        _errStatus = 400;
        std::cout << "Missing Host header." << std::endl;
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
 * @example example of body: username=John&password=1234
 * @note Post must have body, body must fullfill ContentLength, for GET / DELETE, it is not an error to have body
 * @note
 * - body string length has to be as long as the sontent-Length says
 * - here, i can not have this check because the body might be not fully received yet
 * - incomplete body is not an error during streaming)
 */
bool HttpParser::validateBody(){
    if (_req.getMethod() == "POST")
    {
        if (!_isChunked && _req.getBody().size() > _bodyLength){
            _errStatus = 400;
            std::cout << "POST request body length exceeds Content-Length." << std::endl; //here
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
    std::vector<std::string> fields;
    std::string tmp;

    while (ss >> tmp)
        fields.push_back(tmp);

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
        const std::map<std::string, std::string>& headers = _req.getHeaders();

        std::map<std::string, std::string>::const_iterator teIt = headers.find("transfer-encoding");
        if (teIt != headers.end() && teIt->second.find("chunked") != std::string::npos){
            _isChunked = true;
            _currentChunkSize = 0;
            _currentChunkRead = 0;
            _state = BODY;
            return;
        }
        
        std::map<std::string, std::string>::const_iterator it = headers.find("content-length");
        if (it != headers.end()){
            _bodyLength = std::stoi(it->second);
            _isChunked = false;
            _state = BODY;
        }
        else {
            _state = DONE;
        }
        return;
    }
    size_t dd = headerline.find(":");
    if (dd == std::string::npos){
        return;
    }
    std::string key = headerline.substr(0, dd);
    std::string value = headerline.substr(dd + 1);
    key = trim_space(key);
    value = trim_space(value);
    key = normalizeHeaderKey(key);
    _req.addHeader(key, value);
}

/**
 * @brief parse chunked body of an HTTP request
 *
 * @param pos current position in buffer
 * @return void
 *
 * @note Parses Transfer-Encoding: chunked format
 */
void HttpParser::parseChunkedBody(size_t& pos)
{
    while (pos < _buffer.size())
    {
        if (_currentChunkSize == 0)
        {
            size_t lineEnd = _buffer.find("\r\n", pos);
            if (lineEnd == std::string::npos)
                return;
            
            std::string chunkSizeLine = _buffer.substr(pos, lineEnd - pos);
            pos = lineEnd + 2;
            _currentChunkSize = std::strtoul(chunkSizeLine.c_str(), NULL, 16);
            _currentChunkRead = 0;
            
            if (_currentChunkSize == 0)
            {
                if (pos + 1 < _buffer.size() && _buffer.substr(pos, 2) == "\r\n")
                    pos += 2;
                _state = DONE;
                return;
            }
        }
        
        if (_currentChunkRead < _currentChunkSize)
        {
            size_t available = _buffer.size() - pos;
            size_t toRead = std::min(available, _currentChunkSize - _currentChunkRead);
            std::string newBody = _req.getBody();
            newBody += _buffer.substr(pos, toRead);
            _req.setBody(newBody);
            pos += toRead;
            _currentChunkRead += toRead;
    
            if (_currentChunkRead < _currentChunkSize)
                return;
        }
        
        if (_currentChunkRead == _currentChunkSize)
        {
            if (pos + 1 >= _buffer.size()){
                return;
            }
            if (_buffer.substr(pos, 2) == "\r\n")
            {
                pos += 2;
                _currentChunkSize = 0;
                _currentChunkRead = 0;
            }
            else
            {
                _state = ERROR;
                return;
            }
        }
    }
}

/**
 * @brief parse body of an HTTP request
 *
 * @param str headerline
 * @return void
 *
 * @note iss : in stringstream
 */
void HttpParser::parseBody(size_t& pos)
{
    size_t              available = _buffer.size() - pos;
    const std::string&  curBody = _req.getBody();
    size_t              toRead = std::min(available, _bodyLength - curBody.size());
    std::string         newBody = _req.getBody();     

    newBody += _buffer.substr(pos, toRead);             
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
    while (_state != DONE)
    {
        if (_state < BODY)
        {
            size_t end = _buffer.find("\r\n", pos);
            if (end == std::string::npos)
                return HttpRequest();
            std::string line = _buffer.substr(pos, end - pos);
            pos = end + 2;
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            if (_state == START_LINE){
                parseStartLine(line);
                if (_state == ERROR)
                    return HttpRequest();
            }
            if (_state == HEADERS)
                parseHeaderLine(line);
        }
        if (_state == BODY){
            if (_isChunked)
                parseChunkedBody(pos);
            else
                parseBody(pos);
            break;
        }
    }
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
