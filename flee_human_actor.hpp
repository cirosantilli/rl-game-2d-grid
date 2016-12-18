#ifndef FLEE_HUMAN_ACTOR_HPP
#define FLEE_HUMAN_ACTOR_HPP

#include "actor.hpp"

class Action;
class WorldView;

/// Flee the first human it sees. TODO: closest.
/// Dumb: no understanding of walls.
class FleeHumanActor : public Actor {
    public:
        virtual Action act(const WorldView &worldView);
};

#endif
