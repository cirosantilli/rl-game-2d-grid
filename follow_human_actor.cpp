#include "action.hpp"
#include "follow_human_actor.hpp"
#include "object_view.hpp"
#include "world_view.hpp"

Action FollowHumanActor::act(const WorldView &worldView) {
    Action a;
    for (auto const& objectView : worldView.getObjectViews()) {
        if (objectView->getType() == Object::Type::HUMAN) {
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
