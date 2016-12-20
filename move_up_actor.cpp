#include "action.hpp"
#include "move_up_actor.hpp"

Action MoveUpActor::act(const WorldView&) {
    return Action(Action::MoveX::NONE, Action::MoveY::UP);
}
