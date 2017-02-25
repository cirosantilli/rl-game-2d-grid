#include "config.hpp"

Config::Config(std::unique_ptr<std::map<std::string, std::string>> config) :
    config(std::move(config))
{
    this->fracPlantSpawn = this->getConfigDouble("frac-plant-spawn", 0.0001);
    this->treeFracPlantSpawn = this->getConfigDouble("tree-frac-plant-spawn", 0.005);
}

double Config::getConfigDouble(std::string key, double default_) {
    auto f = this->config->find(key);
    return (f == this->config->end()) ? default_ : std::stod(f->second);
}
