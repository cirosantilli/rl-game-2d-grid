#include "action.hpp"
#include "human_actor.hpp"

Action HumanActor::act(const WorldView &worldView) {
    return Action();
}
bool HumanActor::takesHumanAction() const { return true; }
