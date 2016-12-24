#include "action.hpp"
#include "follow_type_actor.hpp"
#include "object_view.hpp"
#include "world_view.hpp"

FollowTypeActor::FollowTypeActor(Object::Type type) : type(type) {}

Action FollowTypeActor::act(const WorldView &worldView) {
    Action a;
    for (auto const& objectView : worldView.getObjectViews()) {
        if (objectView->getType() == this->type) {
            if (objectView->getX() > 0) {
                a.setMoveX(Action::MoveX::RIGHT);
            } else if (objectView->getX() < 0) {
                a.setMoveX(Action::MoveX::LEFT);
            }
            if (0 < objectView->getY()) {
                a.setMoveY(Action::MoveY::UP);
            } else if (0 > objectView->getY()) {
                a.setMoveY(Action::MoveY::DOWN);
            }
            break;
        }
    }
    return a;
}
