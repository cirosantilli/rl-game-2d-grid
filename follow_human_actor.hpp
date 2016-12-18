#ifndef FOLLOW_HUMAN_ACTOR_HPP
#define FOLLOW_HUMAN_ACTOR_HPP

#include "actor.hpp"

class Action;
class WorldView;

/// Follow the first human it sees. TODO: closest.
/// Dumb: no understanding of walls.
class FollowHumanActor : public Actor {
    public:
        virtual Action act(const WorldView &worldView);
};

#endif
