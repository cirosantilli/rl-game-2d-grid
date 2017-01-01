#ifndef DO_NOTHING_ACTOR_HPP
#define DO_NOTHING_ACTOR_HPP

#include "actor.hpp"

class Action;
class WorldView;

class DoNothingActor : public Actor {
    public:
        virtual Action act(const WorldView &worldView);
        virtual std::string getTypeStr() const;
};

#endif
