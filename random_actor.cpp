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
            x = Action::MoveX::RIGHT;
        case 2:
        break;
    }
    switch (this->uniformUintDistribution(this->prng) % 3) {
        case 0:
        break;
        case 1:
            y = Action::MoveY::DOWN;
        break;
            y = Action::MoveY::UP;
        case 2:
        break;
    }
    return Action(x, y);
}

std::string RandomActor::getTypeStr() const {
    return "RandomActor";
}
