#include "action.hpp"
#include "human_actor.hpp"

Action HumanActor::act(const WorldView&) {
    return Action();
}
bool HumanActor::takesHumanAction() const { return true; }
std::string HumanActor::getTypeStr() const {
    return "HumanActor";
}
