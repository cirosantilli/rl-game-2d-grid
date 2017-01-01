#ifndef FOLLOW_TYPE_ACTOR_HPP
#define FOLLOW_TYPE_ACTOR_HPP

#include "actor.hpp"
#include "object.hpp"

class Action;
class WorldView;

/// Follow the first human it sees. TODO: closest.
/// Dumb: no understanding of walls.
class FollowTypeActor : public Actor {
    public:
        FollowTypeActor(Object::Type type = Object::Type::HUMAN);
        virtual Action act(const WorldView &worldView);
        virtual std::string getTypeStr() const;
    private:
        Object::Type type;
};

#endif
