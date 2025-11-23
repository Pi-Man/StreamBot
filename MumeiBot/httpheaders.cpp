#include "httpheaders.h"

#include <algorithm>

#include "civetweb.h"

static bool stricmp(const std::string & a, const std::string & b) {
    return std::equal(a.cbegin(), a.cend(), b.cbegin(), b.cend(), [](unsigned char a, unsigned char b) {return tolower(a) == tolower(b);});
}

HTTPHeaders::HTTPHeaders() {}

HTTPHeaders::HTTPHeaders(const struct mg_header * headers, const int count) {
    for (int i = 0; i < count; i++) {
        if (stricmp(headers[i].name, "cookies")) {
            cookies = HTTPCookies(headers[i].value);
        }
        else {
            map[headers[i].name] = headers[i].value;
        }
    }
}

bool HTTPHeaders::has(const std::string &key) const {
    return map.find(key) != map.end();
}

bool HTTPHeaders::has(const std::string &&key) const {
    return has(key);
}

std::string &HTTPHeaders::operator[](const std::string &key) {
    return map[key];
}

std::string &HTTPHeaders::operator[](const std::string &&key) {
    return map[key];
}

const std::string &HTTPHeaders::operator[](const std::string &key) const {
    return map.at(key);
}

const std::string &HTTPHeaders::operator[](const std::string &&key) const {
    return map.at(key);
}

HTTPHeaders::operator std::string() const {
    std::string headers;
	for (const std::pair<std::string, std::string> & pair : map) {
		headers += pair.first;
		headers += ": ";
		headers += pair.second;
		headers += "\r\n";
	}
	headers += "\r\n";
    return headers;
}

HTTPCookies::HTTPCookies() {}

HTTPCookies::HTTPCookies(const std::string &cookies) {
    size_t index = 0;
	for (size_t i = 0; i < cookies.length(); i++) {
		if (cookies[i] == ';') {
			size_t j = cookies.find('=', index);
			std::string key = cookies.substr(index, j - index);
			std::string val = cookies.substr(j + 1, i - j - 1);
			map[key] = val;
		}
	}
	size_t j = cookies.find('=', index);
	std::string key = cookies.substr(index, j - index);
	std::string val = cookies.substr(j + 1);
	map[key] = val;
}

HTTPCookies::HTTPCookies(const std::string &&cookies) : HTTPCookies(cookies) {}

HTTPCookies::HTTPCookies(const char *cookies) : HTTPCookies(std::string(cookies)) {}

bool HTTPCookies::has(const std::string & key) const {
    return map.find(key) != map.end();
}
bool HTTPCookies::has(const std::string && key) const {
    return has(key);
}

std::string & HTTPCookies::operator[] (const std::string & key) {
    return map[key];
}
std::string & HTTPCookies::operator[] (const std::string && key) {
    return map[key];
}
const std::string & HTTPCookies::operator[] (const std::string & key) const {
    return map.at(key);
}
const std::string & HTTPCookies::operator[] (const std::string && key) const {
    return map.at(key);
}

HTTPCookies::operator std::string() const {
	std::string cookies;
	for (const std::pair<std::string, std::string> & pair : map) {
		cookies += pair.first;
		cookies += "=";
		cookies += pair.second;
		cookies += ";";
	}
	cookies.pop_back();
	return cookies;
}
