#ifndef FLEE_TYPE_ACTOR_HPP
#define FLEE_TYPE_ACTOR_HPP

#include "actor.hpp"
#include "object.hpp"

class Action;
class WorldView;

/// Flee the first human it sees. TODO: closest.
/// Dumb: no understanding of walls.
class FleeTypeActor : public Actor {
    public:
        FleeTypeActor(Object::Type type = Object::Type::HUMAN);
        virtual Action act(const WorldView &worldView);
        virtual std::string getTypeStr() const;
    private:
        Object::Type type;
};

#endif
