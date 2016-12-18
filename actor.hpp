#ifndef ACTOR_HPP
#define ACTOR_HPP

class Action;
class WorldView;

class Actor {
    public:
        /// Perceive one step of the world.
        /// Update internal mental state.
        /// Return an action for this current world step.
        virtual Action act(const WorldView &worldView) = 0;
        virtual bool takesHumanAction() const;
};

#endif
