#include "guild.h"

static picojson::value parse(const std::string & str) {
    picojson::value val;
    picojson::parse(val, str);
    return val;
}

template<typename T>
static const T & get_or_default(const picojson::object & obj, const std::string & key, const T & def) {
    if (obj.find(key) != obj.end()) {
        puts("key found");
        const picojson::value & val = obj.at(key);
        if (val.is<T>()) {
            puts("type found");
            return val.get<T>();
        }
    }
    return def;
}

Guild::Guild() : id(0), name(""), permissions(0) {}

Guild::Guild(const picojson::value & json) : Guild() {
    if (json.is<picojson::object>()) {
        const picojson::object & guild_obj = json.get<picojson::object>();
        id = get_or_default<int64_t>(guild_obj, "id", 20);
        name = get_or_default<std::string>(guild_obj, "name", "");
        permissions = get_or_default<int64_t>(guild_obj, "permissions", 10);
    }
}

Guild::Guild(const picojson::value && json) : Guild(json) {}

Guild::Guild(const std::string & json) : Guild(parse(json)) {}

Guild::Guild(const std::string && json) : Guild(json) {}
