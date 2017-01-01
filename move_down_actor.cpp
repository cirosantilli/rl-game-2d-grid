#include "action.hpp"
#include "move_down_actor.hpp"

Action MoveDownActor::act(const WorldView&) {
    return Action(Action::MoveX::NONE, Action::MoveY::DOWN);
}

std::string MoveDownActor::getTypeStr() const {
    return "MoveDownActor";
}
