#include "config.hpp"

Config::Config(std::unique_ptr<std::map<std::string, std::string>> config) :
    config(std::move(config))
{
    this->fracBadFruit = this->getConfigDouble("fracBadFruit", 0.05);
    this->fracFruit = this->getConfigDouble("fracBadFruit", 0.02);
    this->fracFruitSkin = this->getConfigDouble("fracFruitSkin", 0.01);
    this->fracFruitSpawn = this->getConfigDouble("fracFruitSpawn", 0.001);
    this->fracGreatFruit = this->getConfigDouble("fracBadFruit", 0.002);
    this->fracRooms = this->getConfigDouble("fracRooms", 0.01);
    this->fracTeleport = this->getConfigDouble("fracTeleport", 0.001);
    this->treeFracFruitSpawn = this->getConfigDouble("treeFracFruitSpawn", 0.005);

    this->showFovActor = this->getConfigString(V{"a", "showFovActor"}, "HumanActor", this->showFovActorGiven);

    this->showFovId = this->getConfigUint(V{"v", "showFovId"}, 0, this->showFovIdGiven);
}

double Config::getConfigDouble(std::string key, double default_) const {
    bool _;
    return getConfigDouble(key, default_, _);
}

double Config::getConfigDouble(std::string key, double default_, bool &given) const {
    return this->getConfigDouble(V{key}, default_, given);
}

double Config::getConfigDouble(const V& keys, double default_) const {
    bool _;
    return getConfigDouble(keys, default_, _);
}

double Config::getConfigDouble(const V& keys, double default_, bool &given) const {
    return this->getConfigHelpers<double>(keys, default_, given, [](std::string s){return std::stod(s);});
}

std::string Config::getConfigString(std::string key, std::string default_) const {
    bool _;
    return getConfigString(key, default_, _);
}

std::string Config::getConfigString(std::string key, std::string default_, bool &given) const {
    return this->getConfigString(V{key}, default_, given);
}

std::string Config::getConfigString(const V& keys, std::string default_) const {
    bool _;
    return getConfigString(keys, default_, _);
}

std::string Config::getConfigString(const V& keys, std::string default_, bool &given) const {
    return this->getConfigHelpers<std::string>(keys, default_, given, [](std::string s){return s;});
}

unsigned int Config::getConfigUint(std::string key, unsigned int default_) const {
    bool _;
    return getConfigUint(key, default_, _);
}

unsigned int Config::getConfigUint(std::string key, unsigned int default_, bool &given) const {
    return this->getConfigUint(V{key}, default_, given);
}

unsigned int Config::getConfigUint(const V& keys, unsigned int default_) const {
    bool _;
    return getConfigUint(keys, default_, _);
}

unsigned int Config::getConfigUint(const V& keys, unsigned int default_, bool &given) const {
    return this->getConfigHelpers<unsigned int>(keys, default_, given, [](std::string s){return std::stoul(s);});
}

template<typename T>
T Config::getConfigHelpers(
    const V& keys,
    T default_,
    bool &given,
    std::function<T(std::string)> str2val
) const {
    given = false;
    for (const auto& key : keys) {
        auto f = this->config->find(key);
        if (f != this->config->end()) {
            given = true;
            return str2val(f->second);
        }
    }
    return default_;
}
