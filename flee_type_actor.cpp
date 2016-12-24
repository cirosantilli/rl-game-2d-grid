#include "action.hpp"
#include "flee_type_actor.hpp"
#include "object_view.hpp"
#include "world_view.hpp"

FleeTypeActor::FleeTypeActor(Object::Type type) : type(type) {}

Action FleeTypeActor::act(const WorldView &worldView) {
    Action a;
    for (auto const& objectView : worldView.getObjectViews()) {
        if (objectView->getType() == Object::Type::HUMAN) {
            if (objectView->getX() < 0) {
                a.setMoveX(Action::MoveX::RIGHT);
            } else {
                a.setMoveX(Action::MoveX::LEFT);
            }
            if (objectView->getY() > 0) {
                a.setMoveY(Action::MoveY::DOWN);
            } else {
                a.setMoveY(Action::MoveY::UP);
            }
            break;
        }
    }
    return a;
}
