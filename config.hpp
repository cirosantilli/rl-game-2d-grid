#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <map>
#include <memory> // unique_ptr
#include <string>

class Config {
    public:
        Config(std::unique_ptr<std::map<std::string, std::string>> config);
        /// Public becase we are lazy to write code to preprocess all configs here:
        /// not neededd for configs that are only used once at startup.
        double getConfigDouble(std::string key, double default_);
        double fracFruitSpawn;
        double treeFracFruitSpawn;
    private:
        std::unique_ptr<std::map<std::string, std::string>> config;
};

#endif
