#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <functional> // function
#include <map>
#include <memory> // unique_ptr
#include <string>
#include <vector>

class Config {
    public:
        typedef std::vector<std::string> V;
        Config(std::unique_ptr<std::map<std::string, std::string>> config);

        double getConfigDouble(std::string key, double default_) const;
        double getConfigDouble(std::string key, double default_, bool &given) const;
        double getConfigDouble(const V& keys, double default_) const;
        double getConfigDouble(const V& keys, double default_, bool &given) const;
        std::string getConfigString(std::string key, std::string default_) const;
        std::string getConfigString(std::string key, std::string default_, bool &given) const;
        std::string getConfigString(const V& keys, std::string default_) const;
        std::string getConfigString(const V& keys, std::string default_, bool &given) const;
        unsigned int getConfigUint(std::string key, unsigned int default_) const;
        unsigned int getConfigUint(std::string key, unsigned int default_, bool &given) const;
        unsigned int getConfigUint(const V& keys, unsigned int default_) const;
        unsigned int getConfigUint(const V& keys, unsigned int default_, bool &given) const;

        /// Public becase we are lazy to write code to preprocess all configs here:
        /// not neededd for configs that are only used once at startup.
        bool
            showFovActorTypeGiven,
            showFovIdGiven
        ;
        double
            /// Bad fruits can also be interpreted as a cost of effort:
            /// wasting energy as you move on diffiult terrain.
            ///
            /// Bad fruits with hugely negative cost, are basically walls
            /// that can be taken down with some effort.
            fracBadFruit,
            fracFruit,
            fracFruitSkin,
            fracFruitSpawn,
            fracGreatFruit,
            fracRooms,
            fracTeleport,
            treeFracFruitSpawn
        ;
        std::string showFovActorType;
        unsigned int showFovId;
    private:
        template<typename T>
        T getConfigHelper(std::string key, T default_, bool &given, std::function<T(std::string)> str2val) const;
        template<typename T>
        T getConfigHelpers(const V& keys, T default_, bool &given, std::function<T(std::string)> str2val) const;
        std::unique_ptr<std::map<std::string, std::string>> config;
};

#endif
