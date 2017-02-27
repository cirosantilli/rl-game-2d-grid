#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <functional> // function
#include <map>
#include <memory> // unique_ptr
#include <string>

class Config {
    public:
        Config(std::unique_ptr<std::map<std::string, std::string>> config);

        double getConfigDouble(std::string key, double default_) const;
        double getConfigDouble(std::string key, double default_, bool &given) const;
        std::string getConfigString(std::string key, std::string default_) const;
        std::string getConfigString(std::string key, std::string default_, bool &given) const;
        unsigned int getConfigUint(std::string key, unsigned int default_) const;
        unsigned int getConfigUint(std::string key, unsigned int default_, bool &given) const;

        /// Public becase we are lazy to write code to preprocess all configs here:
        /// not neededd for configs that are only used once at startup.
        bool
            showFovActorGiven,
            showFovIdGiven
        ;
        double
            fracFruitSpawn,
            fracTeleport,
            treeFracFruitSpawn
        ;
        std::string showFovActor;
        unsigned int showFovId;
    private:
        template<typename T>
        T getConfigHelper(std::string key, T default_, bool &given, std::function<T(std::string)> str2val) const;
        std::unique_ptr<std::map<std::string, std::string>> config;
};

#endif
