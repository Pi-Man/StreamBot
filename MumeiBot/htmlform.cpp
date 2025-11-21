#include "htmlform.h"

#include "civetweb.h"

HTMLForm::HTMLForm() {}

HTMLForm::HTMLForm(const std::string &form)
{
    size_t index = 0;
    char buffer[1024];
	for (size_t i = 0; i < form.length(); i++) {
		if (form[i] == '&') {
			size_t j = form.find('=', index);
			std::string key = form.substr(index, j - index);
			std::string val = form.substr(j + 1, i - j - 1);
            mg_url_decode(key.c_str(), key.length(), buffer, 1024, true);
            key = buffer;
            mg_url_decode(val.c_str(), val.length(), buffer, 1024, true);
            val = buffer;
			map[key] = val;
		}
	}
	size_t j = form.find('=', index);
	std::string key = form.substr(index, j - index);
	std::string val = form.substr(j + 1);
	map[key] = val;
}

HTMLForm::HTMLForm(const std::string && form) : HTMLForm(form) {
}

HTMLForm::HTMLForm(const char * form) : HTMLForm((std::string) form) {
}

bool HTMLForm::has(const std::string & key) const {
    return map.find(key) != map.end();
}
bool HTMLForm::has(const std::string && key) const {
    return has(key);
}

std::string & HTMLForm::operator[] (const std::string & key) {
    return map[key];
}
std::string & HTMLForm::operator[] (const std::string && key) {
    return map[key];
}
const std::string & HTMLForm::operator[] (const std::string & key) const {
    return map.at(key);
}
const std::string & HTMLForm::operator[] (const std::string && key) const {
    return map.at(key);
}

HTMLForm::operator std::string() const {
	std::string form;
	char buffer[1024];
	for (const std::pair<std::string, std::string> & pair : map) {
		mg_url_encode(pair.first.c_str(), buffer, 1024);
		form += buffer;
		form += "=";
		mg_url_encode(pair.second.c_str(), buffer, 1024);
		form += buffer;
		form += "&";
	}
	form.pop_back();
	return form;
}
