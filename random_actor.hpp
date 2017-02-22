#ifndef RANDOM_ACTOR_HPP
#define RANDOM_ACTOR_HPP

#include <random>

#include "actor.hpp"

class Action;
class WorldView;

class RandomActor : public Actor {
    public:
        /// @param[in] random_seed
        ///   Chosen randomly before each run, with decent entropy.
        ///   Must be the only source of randomness of an agent.
        ///   You can use it to seed PRNGs like Mersenne Twister.
        ///   For the sake of reproducibility and science, your program should
        ///   not rely on any other sources of randomness, including:
        ///   - race conditions. Synchronize stuff.
        ///   - /dev/urandom. This seed can be thought as coming from it.
        RandomActor(unsigned int randomSeed);
        virtual Action act(const WorldView &worldView);
        virtual std::string getTypeStr() const;
    private:
        std::mt19937 prng;
        std::uniform_int_distribution<unsigned int> uniformUintDistribution;
};

#endif
