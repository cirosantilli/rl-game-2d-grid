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
std::ostream& operator<<(std::ostream& os, const Action::MoveX& x) {
    std::string s;
    switch(x) {
        case Action::MoveX::NONE:
            s = "NONE";
        break;
        case Action::MoveX::LEFT:
            s = "LEFT";
        break;
        case Action::MoveX::RIGHT:
            s = "RIGHT";
        break;
    }
    return os << s;
}
std::ostream& operator<<(std::ostream& os, const Action::MoveY& y) {
    std::string s;
    switch(y) {
        case Action::MoveY::NONE:
            s = "NONE";
        break;
        case Action::MoveY::DOWN:
            s = "DOWN";
        break;
        case Action::MoveY::UP:
            s = "UP";
        break;
    }
    return os << s;
}
std::ostream& operator<<(std::ostream& os, const Action& a) {
    return os << "Action, x = " << a.getMoveX() << ", y = " << a.getMoveY();
}
