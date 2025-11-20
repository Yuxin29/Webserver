#include "Request.hpp"

/*  ******************************HttpRequest********************************  */
HttpRequest::HttpRequest(const std::string& method, const std::string& requestpath, const std::string& version, const std::string& body, const std::map<std::string, std::string>& requestHeaders)
    :_method(method), _requestPath(requestpath), _version(version), _body(body), _requestHeaders(requestHeaders){
}

HttpRequest::HttpRequest(const HttpRequest& other)
    : _method(other._method), _requestPath(other._requestPath), _version(other._version), _body(other._body), _requestHeaders(other._requestHeaders){
}            

HttpRequest::~HttpRequest(){
}

const std::string HttpRequest::getMethod(){
    return _method;
}

const std::string HttpRequest::getrequestPath(){
    return _requestPath;
}   

const std::string HttpRequest::getVersion(){
    return _version;
}   

const std::map<std::string, std::string>  HttpRequest::getrequestHeaders(){
    return _requestHeaders;
}

const std::string HttpRequest::getBody(){
    return _body;
}
/*  ******************************HttpRequest********************************  */

/*  ******************************HttpRequest Parsing********************************  */
//here is the actual parsing process
//below one is private
void HttpParser::parseStartLine(const std::string& startline){
    //std::string                         _method, _path, _version;
    std::istringstream ss(startline);   //ss : stringstream
    ss >> _method >> _path >> _version;

    if (_method.empty() || _path.empty() || _version.empty())
        throw std::runtime_error("Something missing in http request starting line");
    _state = HEADERS;
}

//below one is private, it will be recalled for a few times.
//example http request with a content lenth
// POST /login HTTP/1.1
// Host: example.com
// User-Agent: curl/7.81.0 
// Content-Type: application/x-www-form-urlencoded 
// Content-Length: 27 
// \r\n
// username=John&password=1234
void HttpParser::parseHeaderLine(const std::string& headerline){
    // first check if headers are done, \r\n will be removed \r\n
    if (headerline.empty()){
        std::map<std::string, std::string>::iterator it = _requestHeaders.find("Content-Length"); // try to find content length in map to see it there is bodu
        //if the abouve find it, iit returns the content length pair
        // if not find, return map.end. end of one step past the last element
        if (it != _requestHeaders.end()){//so this is find it 
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

    key.erase(0, key.find_first_not_of(" \t"));
    key.erase(key.find_last_not_of(" \t") + 1);

    value.erase(0, value.find_first_not_of(" \t"));
    value.erase(value.find_last_not_of(" \t") + 1);

    _requestHeaders[key] = value;
}

//this one is going to called mamy times, basically whenever recv() some new bytes, 
// one call of this only append / parse one chunk of data
// the data might be ramdom pieces, not neccessarily a full line
HttpRequest HttpParser::parseHttpRequest(const std::string& rawLine)
{
    _buffer += rawLine;
    
    // std::istringstream ss(_buffer);
    // std::string         line;
    size_t pos = 0;
    while (_state != DONE ) // there is repeatance here when parseHttpRequest called again, getline removes the \n
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
            {
                parseStartLine(line);
                continue;
            }
            if (_state == HEADERS)
            {
                parseHeaderLine(line);
                if (_state == DONE)
                    break;
                if (_state == BODY)
                    break;
            }
        }
        if (_state == BODY)
        {
            size_t available = _buffer.size() - pos;
            _body += _buffer.substr(pos, available);
            pos += available;
            if (_body.size() >= _bodyLength)
                _state = DONE;
            break;
        }
    }
    if (pos > 0)
        _buffer.erase(0, pos);
    if (_state == DONE)
        return HttpRequest(_method, _path, _version, _body, _requestHeaders);
    return HttpRequest();
}
/*  ******************************HttpRequest Parsing********************************  */

/*  ******************************Response Request********************************  */
HttpResponse::HttpResponse(const std::string& version, const std::string& status, const std::string& phase, const std::string& body, const std::map<std::string, std::string>& responseHeaders)
    :_version(version), _status(status), _phase(phase), _body(body), _responseHeaders(responseHeaders){
}

HttpResponse::HttpResponse(const HttpResponse& other)
    : _version(other._version), _status(other._status), _phase(other._phase), _body(other._body), _responseHeaders(other._responseHeaders){
}          

HttpResponse::~HttpResponse(){
}

const std::string HttpResponse::getVersion(){
    return _version;
}

const std::string HttpResponse::getStatus(){
    return _status;
}   

const std::string HttpResponse::getPhase(){
    return _phase;
}   

const std::string HttpResponse::getBody(){
    return _body;
}

const std::map<std::string, std::string>  HttpResponse::getResponseHeaders(){
    return _responseHeaders;
}
/*  ******************************Response Request********************************  */