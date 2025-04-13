#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <vector>
#include <map>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

struct memory {
    char *response;
    size_t size;
};
     
static size_t cb(void *data, size_t size, size_t nmemb, void *userp) {
  size_t realsize = size * nmemb;
  memory *mem = (memory *)userp;
     
  char *ptr = (char*)realloc(mem->response, mem->size + realsize + 1);
  if(ptr == NULL)
    return 0;  /* out of memory! */
     
  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), data, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;
     
  return realsize;
}

class Request {
public:
    Request();
    Request(const std::string &url, const std::string &method = "GET");
    ~Request();
    void setHeader(const std::string &key, const std::string &value);
    void setBody(const std::string &body);
    void setTimeout(long timeout);
    void setFollowRedirects(bool followRedirects);
    void setUrl(const std::string &url);
    void setMethod(const std::string &method);

    void sendRequest();
    nlohmann::json getResponseJson();
    void clean();

private:
    std::string url;
    std::string method;
    std::map<std::string, std::string> headers;
    std::string body;
    long timeout;
    bool followRedirects;

    CURL *curl;
    struct curl_slist *headerList;
    CURLcode res;
    memory chunk = {nullptr, 0};

    void initCurl();
    void cleanupCurl();
};

#endif // REQUEST_H
