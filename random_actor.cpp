#include <cstdlib>

#include "action.hpp"
#include "random_actor.hpp"

RandomActor::RandomActor(unsigned int randomSeed) {
    this->prng.seed(randomSeed);
}

Action RandomActor::act(const WorldView&) {
    Action::MoveX x = Action::MoveX::NONE;
    Action::MoveY y = Action::MoveY::NONE;
    switch (this->uniformUintDistribution(this->prng) % 3) {
        case 0:
        break;
        case 1:
            x = Action::MoveX::LEFT;
        break;
        case 2:
            x = Action::MoveX::RIGHT;
        break;
    }
    switch (this->uniformUintDistribution(this->prng) % 3) {
        case 0:
        break;
        case 1:
            y = Action::MoveY::DOWN;
        break;
        case 2:
            y = Action::MoveY::UP;
        break;
    }
    return Action(x, y);
}

std::string RandomActor::getTypeStr() const {
    return "RandomActor";
}
