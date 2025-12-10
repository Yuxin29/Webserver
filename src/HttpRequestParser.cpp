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
 * @brief validates the startline of a http request
 *
 * @param HttpParser _method within class HttpRequest nested in HttpParser
 * @return true or false, on false, set the _errStatus the coresponding error code
 *
 * @note exxample of startline: GET /index.html HTTP/1.1
 */
bool HttpParser::validateStartLine()
{
    //  GET, POST, DELETE  ---> 405: mathod not allowed
    if (_req.getMethod() != "GET" && _req.getMethod() != "POST" && _req.getMethod() != "DELETE"){
        _errStatus = 405;
        std::cout << "Method not allowed: " << _req.getMethod() << std::endl;
        return false;
    }
    //  Only accept HTTP/1.1
    if (_req.getVersion() != "HTTP/1.1"){
        _errStatus = 400;
        std::cout << "Invalid HTTP version: " << _req.getVersion() << std::endl;
        return false;
    }
    // Enhancement: path must start with "/"
    if (_req.getPath()[0] != '/'){
        _errStatus = 400;
        std::cout << "Path must not start with '/': " << _req.getPath() << std::endl;
        return false;
    }
    // Enhancement: path cannot be too long check
    if (_req.getPath().size() > 2048){
        _errStatus = 400;        
        std::cout << "Request_URI too long" << _req.getPath() << std::endl;
        return false;
    }
    // Enhancement: path cannot have empty space or controling chars
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

        // Enhancement: the headers can not be too big, like todal length 8192 8KB
        if (totalSize > 8192){
            _errStatus = 431; 
            std::cout << "Total header size to big" << std::endl;
            return false;
        }
        // a single header can not be too long: 431, Request Header Fields Too Large
        if (key.size() > 1024 || value.size() > 4096){
            _errStatus = 431; 
            std::cout << "Header field too large: " << key << std::endl;
            return false;
        }
        // validator key: key cannot has space in it
        for (size_t i = 0; i < key.size(); ++i) {
            if (!isgraph(key[i]) || key[i] == ':') {
                _errStatus = 400;
                std::cout << "Invalid header key: " << key << std::endl;
                return false;
            }
        }
        //validateMandatoryHeaders: loop though all keys, has to find host
        if (key == "Host"){
            if (hasHost){
                _errStatus = 400; 
                std::cout << "Multiple Host headers found." << std::endl;
                return false;
            }
            hasHost = true;
        }
        // validateRepeatingHeaders: loop though all keys, can not have repeating keys, multiple Hosts
        if (seenKeys.count(key)) {
            _errStatus = 400;
            std::cout << "Duplicate header key found: " << key << std::endl;
            return false;
        }
        seenKeys.insert(key);
        // Enhancement: value cannot have empty space or controling chars, 
        // can not have it: Header value must not include empty space or controling chars: gzip, deflate, br, zstd
        // for (size_t i = 0; i < value.size(); ++i){
        //     char c = value[i];
        //     if (c < 31 || c == ' '){
        //         _errStatus = 400;        
        //         std::cout << "Header value must not include empty space or controling chars: " << value << std::endl;
        //         return false;
        //     }
        // }
        //validateContentLength: can not be too long like minus number ---> four hundred  or too big(Payload Too Large) ---> 43113
        if (key == "Content-Length"){
            // can not be empty
            if (value.empty()){
                _errStatus = 400;
                std::cout << "Empty Content-Length value." << std::endl;
                return false;
            }
            // can not start with zero
            if (value.size() > 1 && value[0] == '0'){
                _errStatus = 400;
                std::cout << "Content-Length cannot start with zero." << std::endl;
                return false;
            }
            // can not have non digits
            for (size_t i = 0; i < value.length(); ++i){
                if (!isdigit(value[i])){
                    _errStatus = 400;
                    std::cout << "Non-digit character in Content-Length value." << std::endl;
                    return false;
                }
            }
            // can not be minus
            long long len = atoll(value.c_str()); 
            if (len < 0){
                _errStatus = 400;
                std::cout << "Negative Content-Length value." << std::endl;
                return false;
            }
             // can not be too big, 100 MB, max, ask lin or lucio what should be the max
            if (len > 1024 * 1024 * 100){
                _errStatus = 413;
                std::cout << "Content-Length too large." << std::endl;
                return false;
            }
            _bodyLength = static_cast<size_t>(len);
        }
    }
    // it has to have ont and only one host    // headers["Content-Type"] = "text/html";
    // headers["Content-Length"] = std::to_string(body.size());
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
    // Enhancement body cannot contain null byte
    if (_req.getBody().find('\0') != std::string::npos) {
        _errStatus = 400;
        std::cout << "Body contains null byte." << std::endl;
        return false;
    }
    if (_req.getMethod() == "POST")
    {
        // POST has to have body string and in headers, it has to have content-Length"
        //THIS CAN BE 0
        if (_bodyLength == 0 && !_req.getHeaders().count("Content-Length")){
            _errStatus = 400;
            std::cout << "POST request missing Content-Length header." << std::endl; //here
            return false;
        }
        // body lenth can not be too long: in theory it should not happen
        if (_req.getBody().size() > _bodyLength){
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
    // first check imcomplete
    if (_state != DONE )
        return HttpRequest();
    // then validate 
    if (!validateStartLine() || !validateHeaders() || !validateBody())
        _state = ERROR;
    // if error
    if (_state == ERROR)
        return HttpRequest();
    return _req;
}
