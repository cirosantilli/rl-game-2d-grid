#ifndef ACTOR_HPP
#define ACTOR_HPP

#include <string>

class Action;
class WorldView;

class Actor {
    public:
        /// Perceive one step of the world.
        /// Update internal mental state.
        /// Return an action for this current world step.
        virtual Action act(const WorldView &worldView) = 0;

        /// This is horrendous, but
        /// http://stackoverflow.com/questions/281818/unmangling-the-result-of-stdtype-infoname
        virtual std::string getTypeStr() const = 0;

        virtual bool takesHumanAction() const;
};

#endif
