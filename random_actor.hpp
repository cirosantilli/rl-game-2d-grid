#ifndef RANDOM_ACTOR_HPP
#define RANDOM_ACTOR_HPP

#include "actor.hpp"

class Action;
class WorldView;

class RandomActor : public Actor {
    public:
        virtual Action act(const WorldView &worldView);
};

#endif
