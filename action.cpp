#include "action.hpp"

Action::Action() { this->reset(); }
Action::Action(Action::MoveX moveX, Action::MoveY moveY) :
    moveX(moveX), moveY(moveY) {}
Action::MoveX Action::getMoveX() const { return this->moveX; }
Action::MoveY Action::getMoveY() const { return this->moveY; }
void Action::reset() {
    this->moveX = Action::MoveX::NONE;
    this->moveY = Action::MoveY::NONE;
}
void Action::setMoveX(Action::MoveX x) { this->moveX = x; }
void Action::setMoveY(Action::MoveY y) { this->moveY = y; }
