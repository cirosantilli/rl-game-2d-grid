#include "config.hpp"

Config::Config(std::unique_ptr<std::map<std::string, std::string>> config) :
    config(std::move(config))
{
    this->fracFruitSpawn = this->getConfigDouble("fracFruitSpawn", 0.001);
    this->fracTeleport = this->getConfigDouble("fracTeleport", 0.001);
    this->treeFracFruitSpawn = this->getConfigDouble("treeFracFruitSpawn", 0.005);

    this->showFovActor = this->getConfigString("showFovActor", "HumanActor", this->showFovActorGiven);

    this->showFovId = this->getConfigUint("showFovId", 0, this->showFovIdGiven);
}

double Config::getConfigDouble(std::string key, double default_) const {
    bool _;
    return getConfigDouble(key, default_, _);
}

double Config::getConfigDouble(std::string key, double default_, bool &given) const {
    return this->getConfigHelper<double>(key, default_, given, [](std::string s){return std::stod(s);});
}

std::string Config::getConfigString(std::string key, std::string default_) const {
    bool _;
    return getConfigString(key, default_, _);
}

std::string Config::getConfigString(std::string key, std::string default_, bool &given) const {
    return this->getConfigHelper<std::string>(key, default_, given, [](std::string s){return s;});
}

unsigned int Config::getConfigUint(std::string key, unsigned int default_) const {
    bool _;
    return getConfigUint(key, default_, _);
}

unsigned int Config::getConfigUint(std::string key, unsigned int default_, bool &given) const {
    return this->getConfigHelper<unsigned int>(key, default_, given, [](std::string s){return std::stoul(s);});
}

template<typename T>
T Config::getConfigHelper(
    std::string key,
    T default_,
    bool &given,
    std::function<T(std::string)> str2val
) const {
    auto f = this->config->find(key);
    if (f == this->config->end()) {
        given = false;
        return default_;
    } else {
        given = true;
        return str2val(f->second);
    }
}
