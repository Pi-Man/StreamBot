#ifndef _H_GUILD
#define _H_GUILD

#include <string>

#define PICOJSON_USE_INT64
#include <picojson/picojson.h>

struct Guild {

    int64_t id;

    std::string name;

    int64_t permissions;

    Guild();
    Guild(const picojson::value & json);
    Guild(const picojson::value && json);
    Guild(const std::string & json);
    Guild(const std::string && json);

};

#endif