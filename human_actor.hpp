#ifndef HUMAN_ACTOR_HPP
#define HUMAN_ACTOR_HPP

#include "actor.hpp"

class Action;
class WorldView;

class HumanActor : public Actor {
    public:
        // Return a dummy action. Will not be normally used, as human actions will
        // be generated by the keyboard or a TAS replay input.
        virtual Action act(const WorldView &worldView);
        virtual bool takesHumanAction() const;
};

#endif
