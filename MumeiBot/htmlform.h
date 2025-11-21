#ifndef _H_HTMLFORM
#define _H_HTMLFORM

#include <unordered_map>
#include <string>

struct HTMLForm {

    std::unordered_map<std::string, std::string> map;

    HTMLForm();
    HTMLForm(const std::string & form);
    HTMLForm(const std::string && form);
    HTMLForm(const char * form);

    bool has(const std::string & key) const;
    bool has(const std::string && key) const;
    std::string & operator[] (const std::string & key);
    std::string & operator[] (const std::string && key);
    const std::string & operator[] (const std::string & key) const;
    const std::string & operator[] (const std::string && key) const;

    operator std::string() const;
};

#endif