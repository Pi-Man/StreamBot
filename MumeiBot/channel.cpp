#include "channel.h"

static picojson::value parse(const std::string & str) {
    picojson::value val;
    picojson::parse(val, str);
    return val;
}

template<typename T>
static const T & get_or_default(const picojson::object & obj, const std::string & key, const T & def) {
    if (obj.find(key) != obj.end()) {
        const picojson::value & val = obj.at(key);
        if (val.is<T>()) {
            return val.get<T>();
        }
    }
    return def;
}

Channel::Channel() : id(0), name(""), permissions(0) {}

Channel::Channel(const picojson::value & json) : Channel() {
    if (json.is<picojson::object>()) {
        const picojson::object & channel_obj = json.get<picojson::object>();
        id = atoll(get_or_default<std::string>(channel_obj, "id", "0").c_str());
        name = get_or_default<std::string>(channel_obj, "name", "???");
        type = (Channel::Type) get_or_default<int64_t>(channel_obj, "type", 0);
        permissions = atoll(get_or_default<std::string>(channel_obj, "permissions", "0").c_str());
    }
}

Channel::Channel(const picojson::value && json) : Channel(json) {}

Channel::Channel(const std::string & json) : Channel(parse(json)) {}

Channel::Channel(const std::string && json) : Channel(json) {}
