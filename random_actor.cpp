#include <cstdlib>

#include "action.hpp"
#include "random_actor.hpp"

Action RandomActor::act(const WorldView&) {
    Action::MoveX x;
    Action::MoveY y;
    switch (std::rand() % 3) {
        case 0:
            x = Action::MoveX::LEFT;
        break;
        case 1:
            x = Action::MoveX::NONE;
        break;
        case 2:
            x = Action::MoveX::RIGHT;
        break;
    }
    switch (std::rand() % 3) {
        case 0:
            y = Action::MoveY::DOWN;
        break;
        case 1:
            y = Action::MoveY::NONE;
        break;
        case 2:
            y = Action::MoveY::UP;
        break;
    }
    return Action(x, y);
}
