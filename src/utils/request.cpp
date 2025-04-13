#include "utils/request.h"
#include <iostream>
#include <stdexcept>

Request::Request() : url("127.0.0.1"), method("GET"), timeout(30), followRedirects(true) {
    initCurl();
}

Request::Request(const std::string &url, const std::string &method)
    : url(url), method(method), timeout(30), followRedirects(true) {
    initCurl();
}

Request::~Request() {
    cleanupCurl();
}

void Request::setHeader(const std::string &key, const std::string &value) {
    headers[key] = value;
}

void Request::setBody(const std::string &body) {
    this->body = body;
}

void Request::setTimeout(long timeout) {
    this->timeout = timeout;
}

void Request::setFollowRedirects(bool followRedirects) {
    this->followRedirects = followRedirects;
}

void Request::setUrl(const std::string &url) {
    this->url = url;
}

void Request::setMethod(const std::string &method) {
    this->method = method;
}

void Request::sendRequest() {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, followRedirects ? 1L : 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    if (!body.empty()) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    }

    headerList = nullptr;
    for (const auto &header : headers) {
        std::string headerString = header.first + ": " + header.second;
        headerList = curl_slist_append(headerList, headerString.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        throw std::runtime_error("CURL error: " + std::string(curl_easy_strerror(res)));
    }
}

nlohmann::json Request::getResponseJson() {
    char *ct;
    res = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
    if (res != CURLE_OK) {
        throw std::runtime_error("CURL error: " + std::string(curl_easy_strerror(res)));
    }
    if (strcmp(ct, "application/json") != 0) {
        throw std::runtime_error("Response is not JSON: " + std::string(ct));
    }

    if(chunk.size > 0) {
        nlohmann::json jsonResponse = nlohmann::json::parse(chunk.response);
        return jsonResponse;
    } else {
        throw std::runtime_error("Empty response from server.");
    }
}

void Request::clean() {
    if (chunk.response) {
        free(chunk.response);
        chunk.response = nullptr;
        chunk.size = 0;
    }
    if (headerList) {
        cleanupCurl();
        curl_slist_free_all(headerList);
        headerList = nullptr;
    }
    headers.clear();
    body.clear();
}

void Request::initCurl() {
    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL.");
    }
}

void Request::cleanupCurl() {
    if (curl) {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
}
