#ifndef TELEPORT_OBJECT_HPP
#define TELEPORT_OBJECT_HPP

#include "object.hpp"

/**
 * If you run into a teleport, you exit at another target teleport,
 * in the same direction as you were moving in.
 *
 * Teleports are not necessarily symmetric: re-entering one may lead you
 * to a different location than you came in from initially.
 *
 * You only enter if that place is free (e.g. no walls there),
 * or is something that you can normally walk into (e.g. a plant for a plant eater).
 *
 * If the exit is another teleport, you can't enter. This prevents infinite loops.
 *
 * You can't see what lies beyond a teleport without entering it.
 * */
class TeleportObject : public Object {
    public:
        TeleportObject(
            unsigned int x,
            unsigned int y,
            TeleportObject *destination = nullptr
        );
        TeleportObject* getDestination();
        void setDestination(TeleportObject *destination);
    private:
        TeleportObject *destination;
};

#endif
