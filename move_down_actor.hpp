#ifndef MOVE_DOWN_ACTOR_HPP
#define MOVE_DOWN_ACTOR_HPP

#include "actor.hpp"

class Action;
class WorldView;

class MoveDownActor : public Actor {
    public:
        virtual Action act(const WorldView &worldView);
        virtual std::string getTypeStr() const;
};

#endif
