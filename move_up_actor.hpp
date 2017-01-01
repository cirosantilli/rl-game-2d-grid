#ifndef MOVE_UP_ACTOR_HPP
#define MOVE_UP_ACTOR_HPP

#include "actor.hpp"

class Action;
class WorldView;

class MoveUpActor : public Actor {
    public:
        virtual Action act(const WorldView &worldView);
        virtual std::string getTypeStr() const;
};

#endif
