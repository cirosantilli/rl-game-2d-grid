#include "config.hpp"

Config::Config(std::unique_ptr<std::map<std::string, std::string>> config) :
    config(std::move(config))
{
    this->fracFruitSpawn = this->getConfigDouble("frac-fruit-spawn", 0.0001);
    this->fracTeleport = this->getConfigDouble("frac-teleport", 0.001);
    this->treeFracFruitSpawn = this->getConfigDouble("tree-frac-fruit-spawn", 0.005);
}

double Config::getConfigDouble(std::string key, double default_) {
    auto f = this->config->find(key);
    return (f == this->config->end()) ? default_ : std::stod(f->second);
}
