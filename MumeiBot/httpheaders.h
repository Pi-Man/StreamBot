#ifndef _H_HTTPHEADERS
#define _H_HTTPHEADERS

#include <string>
#include <unordered_map>

struct HTTPCookies {

    std::unordered_map<std::string, std::string> map;

    HTTPCookies();
    HTTPCookies(const std::string & cookies);
    HTTPCookies(const std::string && cookies);
    HTTPCookies(const char * cookies);

    bool has(const std::string & key) const;
    bool has(const std::string && key) const;
    std::string & operator[] (const std::string & key);
    std::string & operator[] (const std::string && key);
    const std::string & operator[] (const std::string & key) const;
    const std::string & operator[] (const std::string && key) const;

    operator std::string() const;
};

struct HTTPHeaders {

    HTTPCookies cookies;

    std::unordered_map<std::string, std::string> map;

    HTTPHeaders();
    HTTPHeaders(const struct mg_header * headers, const int count);

    bool has(const std::string & key) const;
    bool has(const std::string && key) const;
    std::string & operator[] (const std::string & key);
    std::string & operator[] (const std::string && key);
    const std::string & operator[] (const std::string & key) const;
    const std::string & operator[] (const std::string && key) const;

    operator std::string() const;
};

#endif